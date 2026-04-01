/*****************************************************************************\
|   === stp.c : 2026 ===                                                      |
|                                                                             |
|    profanOS package manager based on STP protocol                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <poll.h>

#if defined(__profanOS__)
  #include <modules/panda.h>
  #include <profan/md5.h>
  #include <sys/wait.h>
  #include <profan.h>
#elif defined(__linux__)
  #include <sys/ioctl.h> // terminal size for progress bar
#endif

// client setings (editable)
#define DEFAULT_IP "asqel.ddns.net:42024"
#define RECV_TIMEOUT_MS 1000  // stop waiting for a server response after this delay
#define MAX_RETRY_COUNT 4     // give up after this many retries (after a timeout)
#define FAST_DL_ONCE    16    // ask for 16 parts at once in pkg_download

#define PATH_TEMP "/tmp/stp"     // temporary directory for downloads and extraction
#define PATH_STP  "/zada/stp"    // path used as prefix for PKG_LIST and remove scripts
#define PKG_RMDIR "remove"       // directory in PATH_STP to save remove scripts
#define PKG_LIST  "pkg_list.csv" // file in PATH_STP to save intalled package list

#define PATH_UNZIP   "/bin/f/bsdunzip.elf"
#define PATH_OLIVINE "/bin/f/olivine.elf"
#define PATH_RM      "/bin/c/rm.elf"

#ifndef __profanOS__
  #undef PATH_STP
  #define PATH_STP "./stp_data"
#endif

// protocol types
#define GET_ID          0x01
#define GET_ID_RSP      0x02
#define GET_INFO        0x03
#define GET_INFO_RSP    0x04
#define GET_MAX         0x05
#define GET_MAX_RSP     0x06
#define READ_PART       0x07
#define READ_PART_RSP   0x08
#define GET_DEP         0x09
#define GET_DEP_RSP     0x0A

// protocol error codes
#define ERR_UNKNOWN_REQUEST 0xFFFF
#define ERR_UPDATING        0xFF00
#define ERR_INV_ID          0xFF01
#define ERR_OUT_RANGE       0xFF03
#define ERR_TOO_LONG        0xFF04
#define ERR_FAIL            0xFF05

// protocol limits
#define STP_PKT_SIZE        1300
#define STP_MAX_DESC_SIZE   1000
#define STP_MAX_DEPS        (STP_PKT_SIZE - 8) / 8
#define STP_MAX_PART_SIZE   (STP_PKT_SIZE - 8)
#define STP_MAX_NAME_SIZE   64

// XID and TYPE decoding macros
#define R64_TO_XID(r)  ((r) & 0xFFFFFFFFFF)
#define R64_TO_TYPE(r) (((r) >> 48) & 0xFFFF)
#define TYPE_IS_ERR(t) (((t) & 0xFF00) == 0xFF00)

typedef struct {    // used by pkg_get_info
    char name[STP_MAX_NAME_SIZE];
    uint64_t file_size;
    uint32_t version;
    uint8_t md5[16];
    char desc[STP_MAX_DESC_SIZE];
} stp_info_t;

typedef struct {    // used by pkg_download
    uint32_t packets_lost;
    uint32_t packets_recv;
    uint32_t total_ms;
} download_stat_t;

typedef struct {    // used by cmd_install_dl
    int64_t id;
    char name[STP_MAX_NAME_SIZE];
} id_name_pair_t;

download_stat_t g_alltime_dl_stat;

const char *G_SERVER_ADDR = NULL;
int G_TERMINAL_WIDTH = 80;
int G_DOWNLOAD_MUTE = 0;
int G_SOCKET_FD;

/*******************************************
 *                                        *
 *             FILE UTILITIES             *
 *                                        *
********************************************/

static int remove_full_dir(const char *path) {
    #ifdef __profanOS__
        runtime_args_t args = {
            PATH_RM,
            profan_wd_path(),
            3,
            (char *[]) {
                "rm",
                "-rf",
                (char *) path,
                NULL
            },
            environ,
            1 // sleep mode
        };

        if (run_ifexist(&args, NULL)) {
            fprintf(stderr, "stp: Failed to run rm for path %s\n", path);
            return -1;
        }

        return 0;
    #else
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
        return system(cmd) == 0 ? 0 : -1;
    #endif
}

#ifdef __profanOS__ // unused in linux
static int move_element(const char *src, const char *dst) {
    if (rename(src, dst) == 0)
        return 0;

    FILE *in = fopen(src, "rb");
    if (!in)
        return 1;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 1;
    }

    char buffer[8192];
    size_t bytes;
    int ret = 0;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, bytes, out) != bytes) {
            ret = 1;
            break;
        }
    }

    if (ferror(in))
        ret = 1;

    fclose(in);
    fclose(out);

    return ret;
}
#endif

/*******************************************
 *                                        *
 *             PROTOCOL UTILS             *
 *                                        *
********************************************/

static char *error_to_str(uint16_t error) {
    if (error >> 8 != 0xFF)
        return "Success";
    switch (error) {
        case ERR_UNKNOWN_REQUEST:
            return "Invalid request";
        case ERR_UPDATING:
            return "Package is being updated";
        case ERR_INV_ID:
            return "Invalide package id";
        case ERR_OUT_RANGE:
            return "Offset out of range";
        case ERR_TOO_LONG:
            return "Package part too long";
        case ERR_FAIL:
            return "Internal server failure";
        default:
            return "Unknown error";
    }
}

static uint64_t get_random_xid(void) {
    return (((uint64_t) rand() << 32) | rand()) & 0xFFFFFFFFFF;
}

static void purge_receive_buffer(void) {
    // [probably useless] purge the buffer to read only the new response

    struct pollfd pfd = { .fd = G_SOCKET_FD, .events = POLLIN };
    uint8_t tmp[STP_PKT_SIZE];

    while (poll(&pfd, 1, 0) > 0) {
        recv(G_SOCKET_FD, tmp, sizeof(tmp), 0);
    }
}

static int timeout_recv(uint8_t *buf, size_t buf_size) {
    // returns -2 on timeout or the normal recv return value

    struct pollfd pfd = { .fd = G_SOCKET_FD, .events = POLLIN };

    int poll_ret = poll(&pfd, 1, RECV_TIMEOUT_MS);

    if (poll_ret == -1)
        return -1;
    else if (poll_ret == 0)
        return -2;

    return recv(G_SOCKET_FD, buf, buf_size, 0);
}

#define RETERR(...) return (fprintf(stderr, __VA_ARGS__), -1)
#define GOTOERR(section, ...) do {fprintf(stderr, __VA_ARGS__); goto section;} while(0)

static int stp_sarap(uint8_t *buf, int buf_len, uint16_t type) {
    // sarap: send and receive and process

    int retry_count = 0;

    send_and_wait:
    uint64_t xid = get_random_xid();
    uint64_t r = type;

    memcpy(buf, &xid, 6);
    memcpy(buf + 6, &r, 2);

    purge_receive_buffer();

    if (send(G_SOCKET_FD, buf, buf_len, 0) == -1)
        RETERR("stp: send error: %m\n");

    wait_for_response:
    int rlen = timeout_recv(buf, 1300);

    if (rlen == -2) {
        if (retry_count >= MAX_RETRY_COUNT)
            RETERR("stp: recv timeout, max retry count reached\n");
        fprintf(stderr, "stp: recv timeout, retrying... (%d/%d)\n",
                    retry_count + 1, MAX_RETRY_COUNT);
        retry_count++;
        goto send_and_wait;
    }

    if (rlen < 0)
        RETERR("stp: [protocol err] recv error: %m\n");

    if (rlen < 8)
        RETERR("stp: [protocol err] recv too short\n");

    memcpy(&r, buf, 8);

    if (R64_TO_XID(r) != xid) {
        fprintf(stderr, "stp: [protocol warn] wrong xid, waiting for response again\n");
        goto wait_for_response;
    }

    uint16_t resp_type = R64_TO_TYPE(r);

    if (TYPE_IS_ERR(resp_type))
        RETERR("stp: [protocol err] error response received: %s (0x%04x)\n", error_to_str(resp_type), resp_type);

    if (resp_type != type + 1)
        RETERR("stp: [protocol err] unexpected response type %x\n", resp_type);

    return rlen;
}

/*******************************************
 *                                        *
 *   STP CLIENT SIDE PROTOCOL FUNCTIONS   *
 *                                        *
********************************************/

int64_t pkg_get_id(const char *name) {
    if (strlen(name) > STP_MAX_NAME_SIZE)
        return 0; // too long to exist, 0 == not found

    uint64_t r;
    uint8_t buf[STP_PKT_SIZE];
    memset(buf, 0, 8 + STP_MAX_NAME_SIZE);

    int s = strlen(name);
    memcpy(buf + 8, name, s);

    int rlen = stp_sarap(buf, 8 + STP_MAX_NAME_SIZE, GET_ID);
    if (rlen < 0)
        return -1;

    if (rlen != 16)
        RETERR("stp: [protocol err] recv wrong length\n");

    memcpy(&r, buf + 8, 8);
    return r;
}

int64_t pkg_get_info(int64_t id, stp_info_t *info_buf) {
    // returns 0 on success, -1 on error

    uint8_t buf[STP_PKT_SIZE];
    memcpy(buf + 8, &id, 8);

    int rlen = stp_sarap(buf, 16, GET_INFO);
    if (rlen < 0)
        return -1;

    if (rlen < STP_MAX_NAME_SIZE + 20) // 20 = 8 (id) + 8 (file_size) + 4 (version) + 16 (md5)
        RETERR("stp: [protocol err] recv too short\n");

    memcpy(info_buf->name, buf + 8, STP_MAX_NAME_SIZE);
    memcpy(&info_buf->file_size, buf + 8 + STP_MAX_NAME_SIZE, 8);
    memcpy(&info_buf->version, buf + 16 + STP_MAX_NAME_SIZE, 4);
    memcpy(info_buf->md5, buf + 20 + STP_MAX_NAME_SIZE, 16);

    int desc_len = rlen - (STP_MAX_NAME_SIZE + 36);
    if (desc_len > STP_MAX_DESC_SIZE - 1)
        desc_len = STP_MAX_DESC_SIZE - 1;

    memcpy(info_buf->desc, buf + 36 + STP_MAX_NAME_SIZE, desc_len);

    info_buf->desc[desc_len] = '\0';

    return 0;
}

int pkg_get_deps(int64_t id, int64_t *dep_buf, size_t buf_size) {
    // returns number of dependencies

    uint8_t buf[STP_PKT_SIZE];
    memcpy(buf + 8, &id, 8);

    int rlen = stp_sarap(buf, 16, GET_DEP);
    if (rlen < 0)
        return -1;

    int num_deps = (rlen - 8) / 8;

    if (num_deps > (int) buf_size)
        num_deps = (int) buf_size;

    for (int i = 0; i < num_deps; i++) {
        int64_t dep_id;
        memcpy(&dep_id, buf + 8 + i * 8, 8);
        dep_buf[i] = dep_id;
    }

    return num_deps;
}

int64_t pkg_get_max_id(void) {
    uint8_t buf[STP_PKT_SIZE];

    int rlen = stp_sarap(buf, 8, GET_MAX);
    if (rlen < 0)
        return -1;

    if (rlen != 16)
        RETERR("stp: [protocol err] recv wrong length\n");

    uint64_t max_id;
    memcpy(&max_id, buf + 8, 8);
    return max_id;
}

/*******************************************
 *                                        *
 *   STP CLIENT SIDE DOWNLOAD FUNCTIONS   *
 *                                        *
********************************************/

static int download_send(uint64_t xid, int64_t id, int64_t offset, int64_t file_size) {
    uint8_t buf[STP_PKT_SIZE];
    uint64_t r = READ_PART;

    int part_size = (int) file_size - offset;
    if (part_size > STP_MAX_PART_SIZE)
        part_size = STP_MAX_PART_SIZE;

    memcpy(buf, &xid, 6);
    memcpy(buf + 6, &r, 2);
    memcpy(buf + 8, &id, 8);
    memcpy(buf + 16, &offset, 8);
    memcpy(buf + 24, &part_size, 2);

    if (send(G_SOCKET_FD, buf, 26, 0) == -1)
        RETERR("stp: [protocol err] send error: %m\n");

    return 0;
}

static int download_check_md5(const char *file, uint8_t *expected_md5) {
    #ifdef __profanOS__
        // returns 0 if md5 matches, -1 if not
        uint8_t result[16];

        FILE *f = fopen(file, "rb");

        if (!f)
            RETERR("stp: Failed to open file for md5 check\n");

        if (md5_stream(f, result) == -1) {
            fclose(f);
            RETERR("stp: Failed to compute md5\n");
        }

        fclose(f);

        return memcmp(result, expected_md5, 16) == 0 ? 0 : -1;
    #else
        char cmd[512];
        strcpy(cmd, "echo \"");

        for (int i = 0; i < 16; i++)
            sprintf(cmd + 6 + i * 2, "%02x", expected_md5[i]);

        strcat(cmd, "  ");
        strcat(cmd, file);
        strcat(cmd, "\" | md5sum -c - >/dev/null 2>&1");

        return system(cmd) == 0 ? 0 : -1;
    #endif
}

static void download_print_progress(int64_t received, stp_info_t *info) {
    int bar_width = G_TERMINAL_WIDTH / 2;
    int progress = (int) ((received * bar_width) / info->file_size);
    printf("\r%6.2f%% [", (double) received * 100 / info->file_size);
    for (int i = 0; i < bar_width; i++) {
        if (i < progress)
            printf("=");
        else if (i == progress)
            printf(">");
        else
            printf(" ");
    }
    printf("] %s", info->name);
    fflush(stdout);
}

int pkg_download(int64_t id, const char *dest_path, stp_info_t *info, download_stat_t *dl_stat) {
    uint8_t buf[STP_PKT_SIZE];
    uint64_t received_bytes = 0, offset = 0;

    if (FAST_DL_ONCE > 255)
        RETERR("FAST_DL_ONCE must be < 256\n");

    // open the local file for writing
    FILE *f = fopen(dest_path, "wb");
    if (!f)
        RETERR("stp: Failed to open local file for writing\n");

    // set the final size
    int fd = fileno(f);
    if (fd == -1 || ftruncate(fd, info->file_size) != 0) {
        fclose(f);
        RETERR("stp: Failed to set local file size\n");
    }

    char received_parts[FAST_DL_ONCE]; // bitmap to track received parts

    uint32_t start_time = 0;

    if (dl_stat) {
        memset(dl_stat, 0, sizeof(download_stat_t));
        struct timeval tv;
        gettimeofday(&tv, NULL);
        start_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    while (offset < info->file_size) {
        int64_t part_offset = offset;
        int to_wait;

        memset(received_parts, 0, sizeof(received_parts));

        // we use the last bytes of the XID to identify the part in the response
        uint64_t xid = get_random_xid() & 0xFFFFFFFF00;

        // receive responses
        int retry_count = 0;

        for (to_wait = 0; to_wait < FAST_DL_ONCE && offset + to_wait * STP_MAX_PART_SIZE < info->file_size; to_wait++);
        send_and_wait:

        // send requests for the parts
        for (int i = 0; i < FAST_DL_ONCE && offset < info->file_size; i++) {
            if (!received_parts[i] && download_send(xid + i, id, offset, info->file_size))
                goto error;

            offset += STP_MAX_PART_SIZE;
        }

        while (to_wait > 0) {
            int rlen = timeout_recv(buf, sizeof(buf));

            if (rlen == -2) {
                if (retry_count >= MAX_RETRY_COUNT)
                    GOTOERR(error, "stp: recv timeout, max retry count reached\n");
                retry_count++;
                if (dl_stat)
                    dl_stat->packets_lost += to_wait;
                offset = part_offset; // reset offset to resend the same parts
                goto send_and_wait;
            }

            if (rlen < 0)
                GOTOERR(error, "stp: recv error: %m\n");

            if (rlen < 8)
                GOTOERR(error, "stp: [protocol err] recv too short\n");

            uint64_t r;
            memcpy(&r, buf, 8);
            uint16_t resp_type = R64_TO_TYPE(r);

            if (TYPE_IS_ERR(resp_type))
                GOTOERR(error, "stp: [protocol err] error response received: %s (0x%04x)\n",
                            error_to_str(resp_type), resp_type);

            if (resp_type != READ_PART_RSP)
                GOTOERR(error, "stp: [protocol err] unexpected response type 0x%04x\n", resp_type);

            int part_index = R64_TO_XID(r) & 0xFF;

            if ((R64_TO_XID(r) & 0xFFFFFFFF00) != xid || part_index >= FAST_DL_ONCE || received_parts[part_index])
                continue; // probably an old response timed out

            int expected_len = info->file_size - (part_offset + part_index * STP_MAX_PART_SIZE);
            if (expected_len > STP_MAX_PART_SIZE)
                expected_len = STP_MAX_PART_SIZE;
            expected_len += 8; // +8 for the header

            if (rlen != expected_len)
                GOTOERR(error, "stp: [protocol err] recv wrong length %d (expected %d)\n", rlen, expected_len);

            // write the data to the file at the correct offset
            if (fseek(f, part_offset + part_index * STP_MAX_PART_SIZE, SEEK_SET) != 0)
                GOTOERR(error, "stp: Failed to seek in local file\n");

            rlen -= 8;

            if (fwrite(buf + 8, 1, rlen, f) != (size_t) rlen)
                GOTOERR(error, "stp: Failed to write to local file\n");

            received_bytes += rlen;
            if (!G_DOWNLOAD_MUTE)
                download_print_progress(received_bytes, info);

            if (dl_stat)
                dl_stat->packets_recv++;

            received_parts[part_index] = 1;
            to_wait--;
        }
    }

    if (!G_DOWNLOAD_MUTE)
        printf("\n");

    if (received_bytes != info->file_size)
        GOTOERR(error, "stp: error: received %"PRId64" bytes, expected %"PRId64"\n",
                    received_bytes, info->file_size);

    fflush(f); // write before checking sum

    if (download_check_md5(dest_path, info->md5))
        GOTOERR(error, "stp: md5 check failed for downloaded file\n");

    if (dl_stat) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        dl_stat->total_ms = (tv.tv_sec * 1000 + tv.tv_usec / 1000) - start_time;
    }

    fclose(f);
    return 0;

    error:
    fclose(f);
    remove(dest_path);
    return -1;
}

/*******************************************
 *                                        *
 *     LOCAL PACKAGE LIST MANAGEMENT      *
 *                                        *
********************************************/

typedef struct {
    char *name;
    uint32_t version;
    int is_a_dep;
    char **deps;
} local_pkg_info_t;

local_pkg_info_t **G_LPL = NULL;

static char *trim_strdup(const char *str) {
    // trim leading and trailing whitespace and duplicate the string

    while (*str && (isspace((unsigned char) *str)))
        str++;

    size_t len = strlen(str);

    while (len > 0 && isspace((unsigned char) str[len - 1]))
        len--;

    char *result = malloc(len + 1);
    if (!result)
        return NULL;

    memcpy(result, str, len);
    result[len] = '\0';
    return result;
}

int lpl_load(void) {
    FILE *f = fopen(PATH_STP "/" PKG_LIST, "r");
    G_LPL = malloc(sizeof(local_pkg_info_t *));

    if (!f) {
        G_LPL[0] = NULL;
        return 0;
    }

    local_pkg_info_t *info;
    char *token, line[1024];
    int line_num = 0;
    int count = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) < 3) // skip empty lines
            continue;

        info = calloc(1, sizeof(local_pkg_info_t));
        line_num++;

        token = strtok(line, ",");
        if (!token)
            goto parse_error;
        info->name = trim_strdup(token);

        token = strtok(NULL, ",");
        if (!token)
            goto parse_error;
        info->version = atoi(token);

        token = strtok(NULL, ",");
        if (!token)
            goto parse_error;
        info->is_a_dep = atoi(token);

        int dep_count = 0;
        while ((token = strtok(NULL, ",")) != NULL) {
            info->deps = realloc(info->deps, (dep_count + 2) * sizeof(char *));
            info->deps[dep_count] = trim_strdup(token);
            dep_count++;
        }
        if (info->deps)
            info->deps[dep_count] = NULL;

        G_LPL = realloc(G_LPL, (count + 2) * sizeof(local_pkg_info_t *));
        G_LPL[count] = info;
        count++;

        continue;
        parse_error:
        free(info->name);
        free(info);
        fprintf(stderr, "stp: " PKG_LIST ": parse error on line %d, skipping\n", line_num);
    }

    G_LPL[count] = NULL;

    fclose(f);
    return 0;
}

int lpl_save_and_free(void) {
    if (!G_LPL)
        return 0; // nothing to save

    FILE *f = fopen(PATH_STP "/" PKG_LIST, "w");
    if (!f) {
        fprintf(stderr, "stp: Failed to open " PATH_STP "/" PKG_LIST " for writing\n");
        return -1;
    }

    for (int i = 0; G_LPL[i]; i++) {
        fprintf(f, "%s,%u,%d", G_LPL[i]->name, G_LPL[i]->version, G_LPL[i]->is_a_dep);
        if (G_LPL[i]->deps) {
            for (int j = 0; G_LPL[i]->deps[j]; j++) {
                fprintf(f, ",%s", G_LPL[i]->deps[j]);
                free(G_LPL[i]->deps[j]);
            }
            free(G_LPL[i]->deps);
        }
        free(G_LPL[i]->name);
        free(G_LPL[i]);
        fprintf(f, "\n");
    }

    free(G_LPL);
    fclose(f);

    return 0;
}

int lpl_is_installed(const char *name) {
    // 0: not installed
    // 1: installed by user
    // 2: installed as a dependency

    if (!G_LPL)
        lpl_load();

    for (int i = 0; G_LPL[i]; i++) {
        if (strcmp(G_LPL[i]->name, name) == 0)
            return G_LPL[i]->is_a_dep ? 2 : 1;
    }

    return 0;
}

int lpl_add(const char *name, uint64_t version, int is_a_dep) {
    if (!G_LPL)
        lpl_load();

    local_pkg_info_t *info = malloc(sizeof(local_pkg_info_t));
    info->name = strdup(name);
    info->version = version;
    info->is_a_dep = is_a_dep;
    info->deps = NULL;

    int count = 0;
    while (G_LPL && G_LPL[count])
        count++;

    G_LPL = realloc(G_LPL, (count + 2) * sizeof(local_pkg_info_t *));
    G_LPL[count] = info;
    count++;
    G_LPL[count] = NULL;

    return 0;
}

int lpl_remove(const char *name) {
    for (int i = 0; G_LPL[i]; i++) {
        if (strcmp(G_LPL[i]->name, name) != 0)
            continue;

        if (G_LPL[i]->deps) {
            for (int j = 0; G_LPL[i]->deps[j]; j++)
                free(G_LPL[i]->deps[j]);
            free(G_LPL[i]->deps);
        }
        free(G_LPL[i]->name);
        free(G_LPL[i]);

        // shift the rest of the list
        int j = i;
        while (G_LPL[j + 1]) {
            G_LPL[j] = G_LPL[j + 1];
            j++;
        }
        G_LPL[j] = NULL;

        return 0;
    }

    return -1;
}

int lpl_add_dep(const char *name, const char *dep_name) {
    for (int i = 0; G_LPL[i]; i++) {
        if (strcmp(G_LPL[i]->name, name) != 0)
            continue;
        int dep_count = 0;
        if (G_LPL[i]->deps) {
            while (G_LPL[i]->deps[dep_count])
                dep_count++;
        }

        G_LPL[i]->deps = realloc(G_LPL[i]->deps, (dep_count + 2) * sizeof(char *));
        G_LPL[i]->deps[dep_count] = strdup(dep_name);
        G_LPL[i]->deps[dep_count + 1] = NULL;
        return 0;
    }
    return -1;
}

const char *lpl_get_dependent(const char *name) {
    // return the name of the first package found that uses
    // the given package name, or NULL if not found

    for (int i = 0; G_LPL[i]; i++) {
        if (!G_LPL[i]->deps)
            continue;
        if (strcmp(G_LPL[i]->name, name) == 0)
            continue; // skip the package itself
        for (int j = 0; G_LPL[i]->deps[j]; j++) {
            if (strcmp(G_LPL[i]->deps[j], name) == 0)
                return G_LPL[i]->name;
        }
    }

    return NULL;
}

int lpl_update_version(const char *name, uint32_t version) {
    for (int i = 0; G_LPL[i]; i++) {
        if (strcmp(G_LPL[i]->name, name) == 0) {
            G_LPL[i]->version = version;
            return 0;
        }
    }
    return -1;
}

int lpl_update_depflag(const char *name, int is_a_dep) {
    for (int i = 0; G_LPL[i]; i++) {
        if (strcmp(G_LPL[i]->name, name) == 0) {
            G_LPL[i]->is_a_dep = is_a_dep;
            return 0;
        }
    }
    return -1;
}

/*******************************************
 *                                        *
 *      UDP INITIALIZATION FUNCTIONS      *
 *                                        *
********************************************/

static int parse_ipandport(const char *str, struct sockaddr_in *addr) {
    // truc.ddns.net:1234 or 127.0.0.1:1234

    char ip[256];
    int port;

    const char *p = strchr(str, ':');
    if (!p) {
        fprintf(stderr, "stp: Invalid server address format (expected ip:port)\n");
        return -1;
    }

    size_t ip_len = p - str;
    if (ip_len >= sizeof(ip)) {
        fprintf(stderr, "stp: IP address too long\n");
        return -1;
    }

    memcpy(ip, str, ip_len);
    ip[ip_len] = '\0';

    port = atoi(p + 1);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "stp: %s: Invalid port\n", p + 1);
        return -1;
    }

    struct hostent *info = gethostbyname(ip);
    if (!info) {
        fprintf(stderr, "stp: %s: Failed to resolve hostname\n", ip);
        return -1;
    }

    addr->sin_family = AF_INET;
    memcpy(&addr->sin_addr, info->h_addr_list[0], 4);
    addr->sin_port = htons(port);

    /* printf("// ip: %d.%d.%d.%d:%d\n",
        (uint8_t)info->h_addr_list[0][0],
        (uint8_t)info->h_addr_list[0][1],
        (uint8_t)info->h_addr_list[0][2],
        (uint8_t)info->h_addr_list[0][3],
        port
    ); */

    return 0;
}

static int setup_connection(void) {
    struct sockaddr_in addr;

    if (parse_ipandport(G_SERVER_ADDR, &addr))
        return 1;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
        return 1;

    G_SOCKET_FD = fd;

    if (connect(fd, (void *)&addr, sizeof(addr))) {
        fprintf(stderr, "stp: Failed to connect to server: %m\n");
        close(fd);
        return 1;
    }

    return 0;
}

int setup(void) {
    #if defined(__profanOS__)
        panda_get_size((uint32_t *)&G_TERMINAL_WIDTH, NULL);
        if (G_TERMINAL_WIDTH == 0)
            G_TERMINAL_WIDTH = 80;
    #elif defined(__linux__)
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        G_TERMINAL_WIDTH = w.ws_col;
    #else
        G_TERMINAL_WIDTH = 80;
    #endif

    mkdir(PATH_STP, 0755);  // ensure stp directory exists

    return setup_connection();
}

/*******************************************
 *                                        *
 *       SUBCOMMANDS IMPLEMENTATION       *
 *                                        *
********************************************/

int cmd_list(void) {
    int64_t num_ids = pkg_get_max_id();

    if (num_ids < 0)
        return 1;

    printf("available packages:\n");

    for (int i = 1; i <= num_ids; i++) {
        stp_info_t info;

        if (pkg_get_info(i, &info) == -1)
            return 1;

        printf("  %s: %s (%"PRId64" KB)\n", info.name, info.desc, info.file_size / 1024);
    }

    return 0;
}

static id_name_pair_t *cmd_install_dl(uint64_t id, const char *from, id_name_pair_t *dl_deps) {
    stp_info_t info;

    if (pkg_get_info(id, &info) == -1)
        return NULL;

    if (from)
        lpl_add_dep(from, info.name);

    int is_installed = lpl_is_installed(info.name);
    if (is_installed) {
        if (from) {
            printf("dependency '%s' of '%s' is already installed, skipping\n", info.name, from);
            return dl_deps;
        }
        if (is_installed == 1)
            fprintf(stderr, "stp: Package '%s' already installed\n", info.name);
        else {
            printf("package '%s' is now marked as user-installed\n", info.name);
            lpl_update_depflag(info.name, 0);
        }
        return dl_deps;
    }

    int64_t deps[STP_MAX_DEPS];
    int num_deps = pkg_get_deps(id, deps, STP_MAX_DEPS);

    if (num_deps == -1)
        return NULL;

    int dl_deps_count = 0;
    if (dl_deps) {
        while (dl_deps[dl_deps_count].id != -1)
            dl_deps_count++;
    }

    dl_deps = realloc(dl_deps, (dl_deps_count + 2) * sizeof(id_name_pair_t));
    strncpy(dl_deps[dl_deps_count].name, info.name, STP_MAX_NAME_SIZE - 1);
    dl_deps[dl_deps_count].name[STP_MAX_NAME_SIZE - 1] = '\0';
    dl_deps[dl_deps_count].id = id;
    dl_deps[++dl_deps_count].id = -1;

    lpl_add(info.name, info.version, from != NULL);

    for (int i = 0; i < num_deps; i++) {
        // printf("get package %"PRId64", dependency of %s\n", deps[i], info.name);
        dl_deps = cmd_install_dl(deps[i], info.name, dl_deps);
        if (dl_deps == NULL)
            return NULL;
    }

    printf("downloading %s v%d: %s (%"PRId64" KB)\n", info.name, info.version, info.desc, info.file_size / 1024);

    download_stat_t dl_stat;

    char dl_path[PATH_MAX];
    snprintf(dl_path, sizeof(dl_path), PATH_TEMP "/%s.zip", info.name);

    if (pkg_download(id, dl_path, &info, &dl_stat) == -1)
        return NULL;

    g_alltime_dl_stat.packets_lost += dl_stat.packets_lost;
    g_alltime_dl_stat.packets_recv += dl_stat.packets_recv;
    g_alltime_dl_stat.total_ms     += dl_stat.total_ms;

    return dl_deps;
}

static int cmd_install_install(const char *name) {
    printf("installing %s\n", name);
    #ifdef __profanOS__

    char dl_path[PATH_MAX];
    char extract_path[PATH_MAX];
    snprintf(dl_path, sizeof(dl_path), PATH_TEMP "/%s.zip", name);
    snprintf(extract_path, sizeof(extract_path), PATH_TEMP "/%s", name);

    // unzip
    runtime_args_t args = {
        PATH_UNZIP,
        profan_wd_path(),
        5,
        (char *[]) {
            PATH_UNZIP,
            "-q",
            dl_path,
            "-d",
            extract_path,
            NULL
        },
        environ,
        1 // sleep mode
    };

    if (run_ifexist(&args, NULL)) {
        fprintf(stderr, "stp: Failed to run unzip for package '%s'\n", name);
        return 1;
    }

    // chdir
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    if (chdir(extract_path) != 0) {
        fprintf(stderr, "stp: Failed to chdir to '%s'\n", name);
        return 1;
    }

    // olivine
    runtime_args_t olivine_args = {
        PATH_OLIVINE,
        profan_wd_path(),
        2,
        (char *[]) {
            PATH_OLIVINE,
            "install.olv",
            NULL
        },
        environ,
        1 // sleep mode
    };

    if (run_ifexist(&olivine_args, NULL)) {
        fprintf(stderr, "stp: Failed to run olivine for package '%s'\n", name);
        return 1;
    }

    // save the remove.olv
    char remove_dest[PATH_MAX];
    snprintf(remove_dest, sizeof(remove_dest), PATH_STP "/" PKG_RMDIR "/%s.olv", name);

    if (move_element("remove.olv", remove_dest)) {
        fprintf(stderr, "stp: Failed to save remove script for package '%s'\n", name);
        return 1;
    }

    // chdir back
    if (chdir(cwd) != 0) {
        fprintf(stderr, "stp: Failed to chdir back\n", name);
        return 1;
    }
    #endif
    return 0;
}

int cmd_install(char **names) {
    int count, r = 0;
    int64_t *ids;

    for (count = 0; names[count]; count++);
    ids = malloc(count * sizeof(int64_t));

    for (int i = 0; i < count; i++) {
        ids[i] = pkg_get_id(names[i]);
        if (ids[i] == 0)
            fprintf(stderr, "stp: Package '%s' not found\n", names[i]);
        else if (ids[i] != -1)
            continue;
        free(ids);
        return 1;
    }

    remove_full_dir(PATH_TEMP);             // clean temp directory before downloading
    mkdir(PATH_TEMP, 0755);                 // ensure tmp directory exists

    #ifdef __profanOS__
    mkdir(PATH_STP "/" PKG_RMDIR, 0755);    // ensure remove script directory exists
    #endif

    memset(&g_alltime_dl_stat, 0, sizeof(g_alltime_dl_stat));

    id_name_pair_t *dl_deps = malloc(sizeof(id_name_pair_t));
    dl_deps[0].id = -1;

    for (int i = 0; i < count; i++) {
        dl_deps = cmd_install_dl(ids[i], NULL, dl_deps);
        if (dl_deps == NULL) {
            r = 1;
            break;
        }
    }

    count = 0;
    while (dl_deps[count].id != -1)
        count++;

    if (r == 0 && count > 0) {
        printf("all downloads complete in %.2f s - %"PRIu32" packets received, %"PRIu32" lost - %.2f MB/s\n",
                (double) g_alltime_dl_stat.total_ms / 1000.0, g_alltime_dl_stat.packets_recv, g_alltime_dl_stat.packets_lost,
                (double) (g_alltime_dl_stat.packets_recv * STP_PKT_SIZE) / (1024 * 1024) / (g_alltime_dl_stat.total_ms / 1000.0));

        for (int i = count - 1; i >= 0; i--)
            r |= cmd_install_install(dl_deps[i].name);
    }

    remove_full_dir(PATH_TEMP);
    free(dl_deps);
    free(ids);

    return r;
}

static int cmd_remove_remove(const char *name) {
    printf("removing %s\n", name);
    #ifdef __profanOS__
    char remove_script[PATH_MAX];
    snprintf(remove_script, sizeof(remove_script), PATH_STP "/" PKG_RMDIR "/%s.olv", name);

    if (access(remove_script, F_OK) != 0) {
        fprintf(stderr, "stp: No remove script found for package '%s'\n", name);
        return 1;
    }

    runtime_args_t args = {
        PATH_OLIVINE,
        profan_wd_path(),
        2,
        (char *[]) {
            PATH_OLIVINE,
            remove_script,
            NULL
        },
        environ,
        1 // sleep mode
    };

    if (run_ifexist(&args, NULL)) {
        fprintf(stderr, "stp: Failed to run remove script for package '%s'\n", name);
        return 1;
    }

    // remove the script after successful execution
    if (remove(remove_script) != 0) {
        fprintf(stderr, "stp: Failed to remove %s\n", remove_script);
        return 1;
    }
    #endif

    return 0;
}

static int cmd_auto_remove(void) {
    if (!G_LPL)
        lpl_load();

    int found = 0;

    for (int i = 0; G_LPL[i]; i++) {
        if (G_LPL[i]->is_a_dep && lpl_get_dependent(G_LPL[i]->name) == NULL) {
            found = 1;
            if (cmd_remove_remove(G_LPL[i]->name))
                return -1;
            if (lpl_remove(G_LPL[i]->name) == 0)
                continue;
            // should not happen
            fprintf(stderr, "stp: Package '%s' not found in local package list\n", G_LPL[i]->name);
            return -1;
        }
    }

    if (found)
        found += cmd_auto_remove();

    return found;
}

int cmd_remove(char **names) {
    const char *dependent;
    int success = 0;

    for (int i = 0; names[i]; i++) {
        int is_installed = lpl_is_installed(names[i]);
    
        if (is_installed == 0)
            fprintf(stderr, "stp: Package '%s' not installed\n", names[i]);
        else if ((dependent = lpl_get_dependent(names[i])) != NULL)
            if (is_installed == 1) {
                printf("package '%s' requires by '%s' is now marked only as a dependency\n", names[i], dependent);
                lpl_update_depflag(names[i], 1);
                continue;
            }
            else
                fprintf(stderr, "stp: Failed to remove '%s' because '%s' uses it\n", names[i], dependent);
        else if (cmd_remove_remove(names[i]))
            ; // error already printed
        else if (lpl_remove(names[i]))
            fprintf(stderr, "stp: Failed to remove '%s' from local package list\n", names[i]);
        else {
            if (success != -1)
                success++;
            continue;
        };
        success = -1;
    }

    if (success < 0)
        return 1;

    int auto_removed = cmd_auto_remove();
    if (auto_removed == -1) {
        fprintf(stderr, "stp: Failed to remove unused dependencies\n");
        return 1;
    }

    if (success == 0) {
        if (auto_removed == 0)
            printf("no unused dependencies found\n");
        else
            printf("successfully removed %d unused dependencies\n", auto_removed);
    } else {
        if (auto_removed == 0)
            printf("successfully removed %d packages, no unused dependencies found\n", success);
        else
            printf("successfully removed %d packages and %d unused dependencies\n", success, auto_removed);
    }

    return 0;
}

int cmd_upgrade(void) {
    if (!G_LPL)
        lpl_load();

    printf("checking for package updates...\n");

    remove_full_dir(PATH_TEMP);             // clean temp directory before downloading
    mkdir(PATH_TEMP, 0755);                 // ensure tmp directory exists

    #ifdef __profanOS__
    mkdir(PATH_STP "/" PKG_RMDIR, 0755);    // ensure remove script directory exists
    #endif

    char dl_path[PATH_MAX];
    int count = 0;

    id_name_pair_t *dl_deps = malloc(sizeof(id_name_pair_t));
    dl_deps[0].id = -1;

    for (int i = 0; G_LPL[i]; i++) {
        int64_t id = pkg_get_id(G_LPL[i]->name);
        if (id == 0) {
            fprintf(stderr, "stp: Package '%s' not found in server, skipping\n", G_LPL[i]->name);
            continue;
        } else if (id == -1)
            return 1;

        stp_info_t info;
        if (pkg_get_info(id, &info) == -1)
            return 1;

        if (info.version <= G_LPL[i]->version)
            continue;

        printf("downloading %s: v%u -> v%u (%"PRId64" KB)\n", G_LPL[i]->name, G_LPL[i]->version, info.version, info.file_size / 1024);

        snprintf(dl_path, sizeof(dl_path), PATH_TEMP "/%s.zip", info.name);
        if (pkg_download(id, dl_path, &info, NULL) == -1)
            return 1;

        // add to install list
        dl_deps = realloc(dl_deps, (count + 2) * sizeof(id_name_pair_t));
        strncpy(dl_deps[count].name, info.name, STP_MAX_NAME_SIZE - 1);
        dl_deps[count].name[STP_MAX_NAME_SIZE - 1] = '\0';
        dl_deps[count].id = info.version; // store the new version for saving to lpl later
        dl_deps[++count].id = -1;
    }

    for (int i = 0; dl_deps[i].id != -1; i++) {
        if (cmd_remove_remove(dl_deps[i].name))
            return 1;
        if (cmd_install_install(dl_deps[i].name))
            return 1;
        lpl_update_version(dl_deps[i].name, dl_deps[i].id);
    }

    remove_full_dir(PATH_TEMP);
    free(dl_deps);

    if (count == 0)
        printf("All packages are up to date\n");
    else
        printf("Successfully upgraded %d packages\n", count);

    return 0;
}

int cmd_get(char **names) {
    for (int i = 0; names[i]; i++) {
        int64_t id = pkg_get_id(names[i]);
        if (id == 0)
            fprintf(stderr, "stp: Package '%s' not found\n", names[i]);
        else if (id == -1)
            return 1;
        else {
            stp_info_t info;
            if (pkg_get_info(id, &info) == -1)
                return 1;

            printf("downloading %s v%d: %s (%"PRId64" KB)\n", info.name, info.version, info.desc, info.file_size / 1024);

            char dl_path[PATH_MAX];
            snprintf(dl_path, sizeof(dl_path), "%s.zip", info.name);

            if (pkg_download(id, dl_path, &info, NULL) == -1)
                return 1;
        }
    }

    return 0;
}

int cmd_info(char **names) {
    int64_t deps[STP_MAX_DEPS];
    stp_info_t info;

    if (!G_LPL)
        lpl_load();

    for (int i = 0; names[i]; i++) {
        local_pkg_info_t *local_info = NULL;

        for (int j = 0; G_LPL[j]; j++) {
            if (strcmp(G_LPL[j]->name, names[i]) != 0)
                continue;
            const char *dependent = lpl_get_dependent(names[i]);
            local_info = G_LPL[j];
        
            printf("LOCAL VERSION ------------\n");
            printf("  name             %s\n", G_LPL[j]->name);
            printf("  version          %u\n", G_LPL[j]->version);
            printf("  installation     %s\n", G_LPL[j]->is_a_dep ? "dependency" : "by user");
            printf("  first used by    %s\n", dependent ? dependent : "N/A");
            break;
        }

        int64_t id = pkg_get_id(names[i]);
        if (id == 0) {
            fprintf(stderr, "stp: %s: package not found\n", names[i]);
            continue;
        } else if (id == -1)
            return 1;

        if (pkg_get_info(id, &info) == -1)
            return 1;

        printf("REMOTE VERSION -----------\n");
        printf("  id               %"PRId64"\n", id);
        printf("  version          %u\n", info.version);
        printf("  desc             %s\n", info.desc);
        printf("  zip size         %"PRId64" KB\n", info.file_size / 1024);

        int num_deps = pkg_get_deps(id, deps, STP_MAX_DEPS);

        if (num_deps == -1)
            return 1;

        if (num_deps == 0 && (!local_info || local_info->deps == NULL))
            continue;

        printf("DEPENDENCIES -----------\n");
        char **deps_list = malloc(num_deps * sizeof(char *));

        for (int j = 0; j < num_deps; j++) {
            if (pkg_get_info(deps[j], &info) == -1) {
                free(deps_list);
                return 1;
            }
            deps_list[j] = strdup(info.name);
        }

        if (local_info && local_info->deps) {
            for (int j = 0; local_info->deps[j]; j++) {
                int found = 0;
                for (int k = 0; k < num_deps; k++) {
                    if (!deps_list[k] || strcmp(local_info->deps[j], deps_list[k]) != 0)
                        continue;
                    free(deps_list[k]);
                    deps_list[k] = NULL; // mark as found
                    found = 1;
                    break;
                }
                printf("  %-16s L%s\n", local_info->deps[j], found ? "+R" : "");
            }
        }

        for (int j = 0; j < num_deps; j++) {
            if (deps_list[j]) {
                printf("  %-16s R\n", deps_list[j]);
                free(deps_list[j]);
            }
        }

        free(deps_list);
    }

    return 0;
}

int cmd_help(void) {
    fputs("Usage: stp [-s server] <command> [args]\n"
            "Commands:\n"
            "  install <pkg1> [pkg2 ...]  Install packages\n"
            "  remove  [pkg1] [pkg2 ...]  Remove packages and unused dependencies\n"
            "  info    <pkg1> [pkg2 ...]  Show package info\n"
            "  get     <pkg1> [pkg2 ...]  Download packages without installing\n"
            "  list                       List available packages\n"
            "  upgrade                    Upgrade installed packages\n"
            "Options:\n"
            "  -h / --help                Show this help message\n"
            "  -s <server>                Specify server address\n",
        stdout);
    return 0;
}

/*******************************************
 *                                        *
 *           COMMAND LINE STUFF           *
 *                                        *
********************************************/

typedef enum {
    CMD_ERROR = -1,
    CMD_INSTALL,
    CMD_REMOVE,
    CMD_INFO,
    CMD_GET,
    CMD_LIST,
    CMD_UPGRADE,
    CMD_HELP
} command_t;

typedef struct {
    command_t cmd;
    int needs_arg;
    const char *str[8];
} cmd_entry_t;

cmd_entry_t commands[] = {
    { CMD_INSTALL, 1, { "install", "i", NULL } },
    { CMD_REMOVE,  2, { "remove", "rm", NULL } },
    { CMD_INFO,    1, { "info", NULL } },
    { CMD_GET,     1, { "get", NULL } },
    { CMD_LIST,    0, { "list", NULL } },
    { CMD_UPGRADE, 0, { "upgrade", "u", NULL } },
    { CMD_HELP,    0, { "--help", "-h", NULL } }
};

command_t parse_args(int argc, char **argv) {
    if (argc < 2)
        return CMD_HELP;

    if (strcmp(argv[1], "-s") == 0) {
        if (argc < 3) {
            fprintf(stderr, "stp: Missing server address after -s\n");
            return CMD_ERROR;
        }
        G_SERVER_ADDR = argv[2];
        argv += 2;

        if (argc < 4)
            return CMD_HELP;
    }

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        for (size_t j = 0; commands[i].str[j]; j++) {
            if (strcmp(argv[1], commands[i].str[j]))
                continue;
            if (commands[i].needs_arg == 1 && !argv[2]) {
                fprintf(stderr, "stp: %s: Missing argument\n", argv[1]);
                return CMD_ERROR;
            } else if (commands[i].needs_arg == 0 && argv[2]) {
                fprintf(stderr, "stp: %s: Too many arguments\n", argv[1]);
                return CMD_ERROR;
            }
            return commands[i].cmd;
        }
    }

    fprintf(stderr, "stp: %s: Unknown command\n", argv[1]);
    return CMD_ERROR;
}

int main(int argc, char **argv) {
    command_t command = parse_args(argc, argv);

    if (command == CMD_ERROR)
        return 1;

    if (command == CMD_HELP) {
        cmd_help();
        return 0;
    }

    if (G_SERVER_ADDR == NULL)
        G_SERVER_ADDR = DEFAULT_IP;
    else
        argv += 2;

    if (setup())
        return 1;

    int ret;

    switch (command) {
        case CMD_LIST:
            ret = cmd_list();
            break;
        case CMD_INSTALL:
            ret = cmd_install(argv + 2);
            break;
        case CMD_REMOVE:
            ret = cmd_remove(argv + 2);
            break;
        case CMD_INFO:
            ret = cmd_info(argv + 2);
            break;
        case CMD_GET:
            ret = cmd_get(argv + 2);
            break;
        case CMD_UPGRADE:
            ret = cmd_upgrade();
            break;
        default:
            fprintf(stderr, "stp: Command not implemented yet\n");
            ret = 1;
            break;
    }

    lpl_save_and_free();

    close(G_SOCKET_FD);
    return ret;
}
