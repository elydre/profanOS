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
#define FAST_DL_ONCE    16    // ask for 16 parts at once in download_pkg
#define TEMP_DIR "/tmp/stp"   // temporary directory for downloads and extraction

#define PATH_UNZIP   "/bin/f/bsdunzip.elf"
#define PATH_OLIVINE "/bin/f/olivine.elf"
#define PATH_RM      "/bin/c/rm.elf"

// protocol types
#define GET_ID          0x01
#define GET_ID_RSP      0x02
#define GET_INFO        0x03
#define GET_INFO_RSP    0x04
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

typedef struct {
    char name[STP_MAX_NAME_SIZE];
    uint64_t file_size;
    uint32_t version;
    uint8_t md5[16];
    char desc[STP_MAX_DESC_SIZE];
} stp_info_t;

typedef struct {
    uint32_t packets_lost;
    uint32_t packets_recv;
    uint32_t total_ms;
} download_stat_t;

int G_TERMINAL_WIDTH = 80;
int G_DOWNLOAD_MUTE = 0;
int G_FD;

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
            fprintf(stderr, "failed to run rm for path %s\n", path);
            return -1;
        }

        return 0;
    #else
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
        return system(cmd) == 0 ? 0 : -1;
    #endif
}

static int move_element(const char *src, const char *dst) {
    if (rename(src, dst) == 0)
        return 0;

    FILE *in = fopen(src, "rb");
    if (!in) {
        fprintf(stderr, "mv: Failed to open source file '%s'\n", src);
        return 1;
    }

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fprintf(stderr, "mv: Failed to open destination file '%s'\n", dst);
        fclose(in);
        return 1;
    }

    char buffer[8192];
    size_t bytes;
    int ret = 0;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, bytes, out) != bytes) {
            fprintf(stderr, "mv: Failed to write to destination file '%s'\n", dst);
            ret = 1;
            break;
        }
    }

    if (ferror(in)) {
        fprintf(stderr, "mv: Failed to read from source file '%s'\n", src);
        ret = 1;
    }

    fclose(in);
    fclose(out);

    return ret;
}

/*******************************************
 *                                        *
 *             PROTOCOL UTILS             *
 *                                        *
********************************************/

char *error_to_str(uint16_t error) {
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

    struct pollfd pfd = { .fd = G_FD, .events = POLLIN };
    uint8_t tmp[STP_PKT_SIZE];

    while (poll(&pfd, 1, 0) > 0) {
        recv(G_FD, tmp, sizeof(tmp), 0);
    }
}

static int timeout_recv(uint8_t *buf, size_t buf_size) {
    // returns -2 on timeout or the normal recv return value

    struct pollfd pfd = { .fd = G_FD, .events = POLLIN };

    int poll_ret = poll(&pfd, 1, RECV_TIMEOUT_MS);

    if (poll_ret == -1)
        return -1;
    else if (poll_ret == 0)
        return -2;

    return recv(G_FD, buf, buf_size, 0);
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

    if (send(G_FD, buf, buf_len, 0) == -1)
        RETERR("[protocol err] send error %d (%s)\n", errno, strerror(errno));

    wait_for_response:
    int rlen = timeout_recv(buf, 1300);

    if (rlen == -2) {
        if (retry_count >= MAX_RETRY_COUNT)
            RETERR("[protocol err] recv timeout, max retry count reached\n");
        fprintf(stderr, "[protocol warn] recv timeout, retrying... (%d/%d)\n",
                    retry_count + 1, MAX_RETRY_COUNT);
        retry_count++;
        goto send_and_wait;
    }

    if (rlen < 0)
        RETERR("[protocol err] recv error %d (%s)\n", errno, strerror(errno));

    if (rlen < 8)
        RETERR("[protocol err] recv too short\n");

    memcpy(&r, buf, 8);

    if (R64_TO_XID(r) != xid) {
        fprintf(stderr, "[protocol warn] wrong xid, waiting for response again\n");
        goto wait_for_response;
    }

    uint16_t resp_type = R64_TO_TYPE(r);

    if (TYPE_IS_ERR(resp_type))
        RETERR("[protocol err] error response received: %s (0x%04x)\n", error_to_str(resp_type), resp_type);

    if (resp_type != type + 1)
        RETERR("[protocol err] unexpected response type %x\n", resp_type);

    return rlen;
}

/*******************************************
 *                                        *
 *   STP CLIENT SIDE PROTOCOL FUNCTIONS   *
 *                                        *
********************************************/

int64_t get_pkg_id(const char *name) {
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
        RETERR("[protocol err] recv wrong length\n");

    memcpy(&r, buf + 8, 8);
    return r;
}

int64_t get_pkg_info(int64_t id, stp_info_t *info_buf) {
    // returns 0 on success, -1 on error

    uint8_t buf[STP_PKT_SIZE];
    memcpy(buf + 8, &id, 8);

    int rlen = stp_sarap(buf, 16, GET_INFO);
    if (rlen < 0)
        return -1;

    if (rlen < STP_MAX_NAME_SIZE + 20) // 20 = 8 (id) + 8 (file_size) + 4 (version) + 16 (md5)
        RETERR("[protocol err] recv too short\n");

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

int get_pkg_deps(int64_t id, int64_t *dep_buf, size_t buf_size) {
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

    if (send(G_FD, buf, 26, 0) == -1)
        RETERR("[protocol err] send error %d\n", errno);

    return 0;
}

static int download_check_md5(const char *file, uint8_t *expected_md5) {
    #ifdef __profanOS__
        // returns 0 if md5 matches, -1 if not
        uint8_t result[16];

        FILE *f = fopen(file, "rb");

        if (!f)
            RETERR("failed to open file for md5 check\n");

        if (md5_stream(f, result) == -1) {
            fclose(f);
            RETERR("failed to compute md5\n");
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

int download_pkg(int64_t id, const char *dest_path, stp_info_t *info, download_stat_t *dl_stat) {
    uint8_t buf[STP_PKT_SIZE];
    uint64_t received_bytes = 0, offset = 0;

    if (FAST_DL_ONCE > 255)
        RETERR("FAST_DL_ONCE must be < 256\n");

    // open the local file for writing
    FILE *f = fopen(dest_path, "wb");
    if (!f)
        RETERR("failed to open local file for writing\n");

    // set the final size
    if (fseek(f, info->file_size - 2, SEEK_SET) != 0 || fwrite("", 1, 1, f) != 1) {
        fclose(f);
        RETERR("failed to set local file size\n");
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
                    GOTOERR(error, "[protocol err] recv timeout, max retry count reached\n");
                retry_count++;
                if (dl_stat)
                    dl_stat->packets_lost += to_wait;
                offset = part_offset; // reset offset to resend the same parts
                goto send_and_wait;
            }

            if (rlen < 0)
                GOTOERR(error, "[protocol err] recv error %d\n", errno);

            if (rlen < 8)
                GOTOERR(error, "[protocol err] recv too short\n");

            uint64_t r;
            memcpy(&r, buf, 8);
            uint16_t resp_type = R64_TO_TYPE(r);

            if (TYPE_IS_ERR(resp_type))
                GOTOERR(error, "[protocol err] error response received: %s (0x%04x)\n",
                            error_to_str(resp_type), resp_type);

            if (resp_type != READ_PART_RSP)
                GOTOERR(error, "[protocol err] unexpected response type 0x%04x\n", resp_type);

            int part_index = R64_TO_XID(r) & 0xFF;

            if ((R64_TO_XID(r) & 0xFFFFFFFF00) != xid || part_index >= FAST_DL_ONCE || received_parts[part_index])
                continue; // probably an old response timed out

            int expected_len = info->file_size - (part_offset + part_index * STP_MAX_PART_SIZE);
            if (expected_len > STP_MAX_PART_SIZE)
                expected_len = STP_MAX_PART_SIZE;
            expected_len += 8; // +8 for the header

            if (rlen != expected_len)
                GOTOERR(error, "[protocol err] recv wrong length %d (expected %d)\n", rlen, expected_len);

            // write the data to the file at the correct offset
            if (fseek(f, part_offset + part_index * STP_MAX_PART_SIZE, SEEK_SET) != 0)
                GOTOERR(error, "failed to seek in local file\n");

            rlen -= 8;

            if (fwrite(buf + 8, 1, rlen, f) != (size_t) rlen)
                GOTOERR(error, "failed to write to local file\n");

            received_bytes += rlen;
            if (!G_DOWNLOAD_MUTE)
                download_print_progress(received_bytes, info);

            if (dl_stat)
                dl_stat->packets_recv++;

            received_parts[part_index] = 1;
            to_wait--;
        }
    }
    printf("\n");

    if (received_bytes != info->file_size)
        GOTOERR(error, "download incomplete: received %"PRId64" bytes, expected %"PRId64"\n",
                    received_bytes, info->file_size);

    fflush(f); // write before checking sum

    if (download_check_md5(dest_path, info->md5))
        GOTOERR(error, "md5 checksum mismatch\n");

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

int get_pkg_list(uint64_t **ids) {
    stp_info_t *info_buf = malloc(sizeof(stp_info_t));

    if (get_pkg_info(0, info_buf) == -1) {
        free(info_buf);
        return -1;
    }

    // download the list
    if (download_pkg(0, "pkg_list.tmp", info_buf, NULL) == -1) {
        free(info_buf);
        return -1;
    }

    // the list is a sequence of 8-byte ids
    int num_ids = info_buf->file_size / 8;
    free(info_buf);

    uint64_t *id_list = malloc(num_ids * 8);

    FILE *f = fopen("pkg_list.tmp", "rb");
    if (!f) {
        free(id_list);
        return -1;
    }

    if (fread(id_list, 8, num_ids, f) != (size_t) num_ids) {
        free(id_list);
        fclose(f);
        return -1;
    }

    fclose(f);
    remove("pkg_list.tmp");

    *ids = id_list;
    return num_ids;
}

/*******************************************
 *                                        *
 *      UDP INITIALIZATION FUNCTIONS      *
 *                                        *
********************************************/

int parse_ipandport(const char *str, struct sockaddr_in *addr) {
    // truc.ddns.net:1234 or 127.0.0.1:1234

    char ip[256];
    int port;

    const char *p = strchr(str, ':');
    if (!p) {
        fprintf(stderr, "stp: expected ip:port format\n");
        return -1;
    }

    size_t ip_len = p - str;
    if (ip_len >= sizeof(ip)) {
        fprintf(stderr, "stp: ip too long\n");
        return -1;
    }

    memcpy(ip, str, ip_len);
    ip[ip_len] = '\0';

    port = atoi(p + 1);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "stp: '%s' invalid port\n", p + 1);
        return -1;
    }

    struct hostent *info = gethostbyname(ip);
    if (!info) {
        fprintf(stderr, "stp: %s: host non trouve\n", str);
        return -1;
    }

    addr->sin_family = AF_INET;
    memcpy(&addr->sin_addr, info->h_addr_list[0], 4);
    addr->sin_port = htons(port);

    printf("// ip: %d.%d.%d.%d:%d\n",
        (uint8_t)info->h_addr_list[0][0],
        (uint8_t)info->h_addr_list[0][1],
        (uint8_t)info->h_addr_list[0][2],
        (uint8_t)info->h_addr_list[0][3],
        port
    );

    return 0;
}

int setup_connection(void) {
    struct sockaddr_in addr;

    if (parse_ipandport(DEFAULT_IP, &addr))
        return 1;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    G_FD = fd;

    if (fd < 0) {
        return 1;
    }

    if (connect(fd, (void *)&addr, sizeof(addr))) {
        fprintf(stderr, "failed to connect to server: %d\n", errno);
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

    return setup_connection();
}

/*******************************************
 *                                        *
 *       SUBCOMMANDS IMPLEMENTATION       *
 *                                        *
********************************************/

int cmd_list(void) {
    uint64_t *ids;
    int num_ids = get_pkg_list(&ids);

    if (num_ids < 0)
        return 1;

    printf("available packages:\n");

    for (int i = 0; i < num_ids; i++) {
        stp_info_t info;

        if (get_pkg_info(ids[i], &info) == -1) {
            free(ids);
            return 1;
        }

        printf("  %s: %s (%"PRId64" KB)\n", info.name, info.desc, info.file_size / 1024);
    }

    free(ids);
    return 0;
}

download_stat_t g_alltime_dl_stat;

typedef struct {
    int64_t id;
    char name[STP_MAX_NAME_SIZE];
} id_name_pair_t;

static id_name_pair_t *cmd_install_dl(uint64_t id, id_name_pair_t *dl_deps) {
    stp_info_t info;

    if (get_pkg_info(id, &info) == -1)
        return NULL;

    int64_t deps[STP_MAX_DEPS];
    int num_deps = get_pkg_deps(id, deps, STP_MAX_DEPS);

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

    for (int i = 0; i < num_deps; i++) {
        for (int j = 0; j < dl_deps_count; j++) {
            if (deps[i] == dl_deps[j].id) {
                fprintf(stderr, "dependency %s of %s already downloaded, skipping\n", dl_deps[j].name, info.name);
                goto next_dep;
            }
        }

        printf("get package %"PRId64", dependency of %s\n", deps[i], info.name);
        dl_deps = cmd_install_dl(deps[i], dl_deps);
        if (dl_deps == NULL)
            return NULL;
        dl_deps_count = 0;
        while (dl_deps[dl_deps_count].id != -1)
            dl_deps_count++;
        next_dep:;
    }

    printf("PACKAGE INFO:\n");
    printf("  id        %"PRId64"\n", id);
    printf("  name      %s\n", info.name);
    printf("  desc      %s\n", info.desc);
    printf("  file_size %"PRId64" KB\n", info.file_size / 1024);
    printf("  version   %u\n", info.version);

    download_stat_t dl_stat;

    char dl_path[PATH_MAX];
    snprintf(dl_path, sizeof(dl_path), TEMP_DIR "/%s.zip", info.name);

    if (download_pkg(id, dl_path, &info, &dl_stat) == -1)
        return NULL;

    g_alltime_dl_stat.packets_lost += dl_stat.packets_lost;
    g_alltime_dl_stat.packets_recv += dl_stat.packets_recv;
    g_alltime_dl_stat.total_ms     += dl_stat.total_ms;

    return dl_deps;
}

static int cmd_install_install(id_name_pair_t *dl_deps) {
    int count = 0;
    while (dl_deps[count].id != -1)
        count++;

    for (int i = count - 1; i >= 0; i--) {
        #ifdef __profanOS__
        printf("installing package %s...\n", dl_deps[i].name);
        char dl_path[PATH_MAX];
        char extract_path[PATH_MAX];
        snprintf(dl_path, sizeof(dl_path), TEMP_DIR "/%s.zip", dl_deps[i].name);
        snprintf(extract_path, sizeof(extract_path), TEMP_DIR "/%s", dl_deps[i].name);

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

        int pid;

        if (run_ifexist(&args, &pid)) {
            fprintf(stderr, "failed to run unzip for package %s\n", dl_deps[i].name);
            return -1;
        }

        // chdir
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));

        if (chdir(extract_path) != 0) {
            fprintf(stderr, "failed to chdir to package %s\n", dl_deps[i].name);
            return -1;
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

        if (run_ifexist(&olivine_args, &pid)) {
            fprintf(stderr, "failed to run olivine for package %s\n", dl_deps[i].name);
            return -1;
        }

        // save the remove.olv file to /zada/stp/XXXXX.olv

        char remove_dest[PATH_MAX];
        snprintf(remove_dest, sizeof(remove_dest), "/zada/stp/remove/%s.olv", dl_deps[i].name);

        if (move_element("remove.olv", remove_dest)) {
            fprintf(stderr, "failed to save remove script for package %s\n", dl_deps[i].name);
            return -1;
        }

        // chdir back
        if (chdir(cwd) != 0) {
            fprintf(stderr, "failed to chdir back after installing package %s\n", dl_deps[i].name);
            return -1;
        }

        #else
        printf("  %d\n", (int) dl_deps[i]);
        printf("  installation not available...\n");
        #endif
    }

    return 0;
}

int cmd_install(char **names) {
    int count, r = 0;
    int64_t *ids;
    
    for (count = 0; names[count]; count++);
    ids = malloc(count * sizeof(int64_t));

    for (int i = 0; i < count; i++) {
        ids[i] = get_pkg_id(names[i]);
        if (ids[i] == 0)
            fprintf(stderr, "package '%s' not found\n", names[i]);
        else if (ids[i] != -1)
            continue;
        free(ids);
        return 1;
    }

    remove_full_dir(TEMP_DIR);  // clean temp directory before downloading
    mkdir(TEMP_DIR, 0755);      // ensure tmp directory exists
    #ifdef __profanOS__
    mkdir("/zada/stp", 0755);           // ensure remove script directory exists
    mkdir("/zada/stp/remove", 0755);   // ensure remove script directory exists
    #endif

    id_name_pair_t *dl_deps = NULL;

    for (int i = 0; i < count; i++) {
        dl_deps = cmd_install_dl(ids[i], dl_deps);
        if (dl_deps == NULL) {
            r = 1;
            break;
        }
    }

    if (r == 0) {
        printf("all downloads complete: %u ms, %"PRIu32" packets received, %"PRIu32" packets lost, %.2f MB/s\n",
                    g_alltime_dl_stat.total_ms, g_alltime_dl_stat.packets_recv, g_alltime_dl_stat.packets_lost,
                    (double) (g_alltime_dl_stat.packets_recv * STP_PKT_SIZE) / (1024 * 1024) / (g_alltime_dl_stat.total_ms / 1000.0));

        r = cmd_install_install(dl_deps);
    }

    remove_full_dir(TEMP_DIR);
    free(dl_deps);
    free(ids);

    return r;
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
    CMD_LIST,
    CMD_UPDATE,
    CMD_HELP
} command_t;

typedef struct {
    command_t cmd;
    int needs_arg;
    const char *str[8];
} cmd_entry_t;

cmd_entry_t commands[] = {
    { CMD_INSTALL, 1, { "install", "i", NULL } },
    { CMD_REMOVE,  1, { "remove", "rm", "r", NULL } },
    { CMD_LIST,    0, { "list", "ls", "l", NULL } },
    { CMD_UPDATE,  0, { "update", "up", "u", NULL } },
    { CMD_HELP,    0, { "help", "--help", "-h", NULL } }
};

command_t parse_args(int argc, char **argv) {
    if (argc < 2)
        return CMD_HELP;

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        for (size_t j = 0; commands[i].str[j]; j++) {
            if (strcmp(argv[1], commands[i].str[j]))
                continue;
            if (commands[i].needs_arg && argc < 3) {
                fprintf(stderr, "command '%s' needs an argument\n", argv[1]);
                return CMD_ERROR;
            } else if (!commands[i].needs_arg && argc > 2) {
                fprintf(stderr, "command '%s' does not take arguments\n", argv[1]);
                return CMD_ERROR;
            }
            return commands[i].cmd;
        }
    }

    fprintf(stderr, "unknown command: %s\n", argv[1]);
    return CMD_ERROR;
}

int main(int argc, char **argv) {
    command_t command = parse_args(argc, argv);

    if (command == CMD_ERROR)
        return 1;

    if (command == CMD_HELP) {
        printf("please ask to pf4 for help :)\n");
        return 0;
    }

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
        default:
            fprintf(stderr, "command not implemented yet\n");
            ret = 1;
            break;
    }

    close(G_FD);
    return ret;
}
