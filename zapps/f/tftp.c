/*****************************************************************************\
|   === tftp.c : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf, libpni

#include <profan/carp.h>
#include <profan/pni.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TFTP_PORT 69
#define ANSI_UPCLEAR "\e[1A\e[2K"

in_port_t src_port, dest_port;
in_addr_t server_ip;
int display_mode = 0;

#define DEBUG(x) if (x != 0) { \
    fprintf(stderr, "Error at line %d\n", __LINE__); \
    exit(1); \
}


/**********************************
 *                               *
 *    TFTP Protocol Functions    *
 *                               *
**********************************/

void send_rrq(const char *filename) {
    uint8_t packet[516];
    uint16_t opcode = htons(1);
    int len = strlen(filename);

    memcpy(packet, &opcode, 2);
    strlcpy((char *) packet + 2, filename, 500);
    strcpy((char *) packet + 3 + len, "octet");

    DEBUG(pni_send(src_port, dest_port, packet, len + 9, (uint8_t *) server_ip));
}

void send_wrq(const char *filename) {
    uint8_t packet[516];
    uint16_t opcode = htons(2);
    int len = strlen(filename);

    memcpy(packet, &opcode, 2);
    strlcpy((char *) packet + 2, filename, 500);
    strcpy((char *) packet + 3 + len, "octet");

    DEBUG(pni_send(src_port, dest_port, packet, len + 9, (uint8_t *) server_ip));
}

void send_data(in_port_t dest, uint16_t block_num, const uint8_t *data, size_t len) {
    uint8_t packet[516];
    uint16_t opcode = htons(3);

    if (len > 512)
        return;

    block_num = htons(block_num);

    memcpy(packet, &opcode, 2);
    memcpy(packet + 2, &block_num, 2);
    memcpy(packet + 4, data, len);

    DEBUG(pni_send(src_port, dest, (uint8_t *) packet, 4 + len, (uint8_t *) server_ip));
}

void send_ack(in_port_t dest, uint16_t block_num) {
    uint8_t packet[4];
    uint16_t tmp;

    tmp = htons(4); // ACK opcode
    memcpy(packet, &tmp, 2);

    tmp = htons(block_num); // Block number
    memcpy(packet + 2, &tmp, 2);

    DEBUG(pni_send(src_port, dest, packet, sizeof(packet), (uint8_t *) server_ip));
}

/**********************************
 *                               *
 *    File Transfer Functions    *
 *                               *
**********************************/

#define ERROR {         \
    free(packet.data);  \
    fclose(file);       \
    return 1;           \
}

int tftp_get_file(const char *filename) {
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        perror("tftp");
        return 1;
    }

    if (display_mode)
        printf("%s: starting transfer...\n", filename);

    pni_packet_t packet;
    packet.data = NULL;

    send_rrq(filename);

    uint16_t expected_block = 1;
    int retries = 0;

    while (1) {
        free(packet.data);

        packet = pni_recv(PNI_RECV_HANGTIME, 1000);

        if (packet.dest_port != src_port || memcmp(packet.src_ip, &server_ip, 4) != 0)
            continue;

        if (packet.len < 4) {
            fprintf(stderr, "tftp: received packet too short\n");
            ERROR;
        }

        uint16_t opcode = ntohs(*(uint16_t *)(packet.data));
        
        if (opcode == 5) {
            fprintf(stderr, "tftp: server error: %s\n", packet.data + 4);
            ERROR;
        } else if (opcode != 3) {
            fprintf(stderr, "tftp: unexpected opcode %d during receive\n", opcode);
            ERROR;
        }

        uint16_t block_num = ntohs(*(uint16_t *)(packet.data + 2));

        if (block_num != expected_block) {
            if (retries >= 10) {
                fprintf(stderr, "tftp: too many retries, aborting transfer\n");
                ERROR;
            }

            if (display_mode)
                printf(ANSI_UPCLEAR "%s: wrong block number, retry...\n", filename);
            
            // Resend ACK for the expected block number
            if (block_num < expected_block)
                send_ack(dest_port, expected_block);
            
            retries++;
            continue;
        }

        retries = 0;

        size_t data_len = packet.len - 4;
        fwrite(packet.data + 4, 1, data_len, file);

        if (display_mode)
            printf(ANSI_UPCLEAR"%s: %d bytes (block %d)\n", filename, block_num * 512, block_num);

        send_ack(packet.src_port, block_num);
        expected_block++;

        if (data_len < 512) {
            printf(ANSI_UPCLEAR"%s: transfer complete, %d bytes\n", filename, ftell(file));
            break;
        }
    }

    free(packet.data);
    fclose(file);
    return 0;
}


int tftp_send_file(const char *fileName) {
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    pni_packet_t packet;
    packet.data = NULL;

    send_wrq(fileName);

    uint16_t block_num = 0;
    uint8_t data[512];

    if (display_mode)
        printf("%s: starting transfer...\n", fileName);

    while (1) {
        free(packet.data);

        if (block_num) {
            size_t read_len = fread(data, 1, 512, file);

            if (read_len == 0) {
                if (display_mode)
                    printf(ANSI_UPCLEAR "%s: transfer complete, %d bytes\n", fileName, ftell(file));
                break; // End of file reached
            }

            send_data(packet.src_port, block_num, data, read_len);
        }

        packet = pni_recv(0);

        if (packet.dest_port != src_port || memcmp(packet.src_ip, &server_ip, 4) != 0)
            continue;

        if (packet.len < 4) {
            fprintf(stderr, "tftp: received packet too short\n");
            ERROR;
        }

        uint16_t opcode = ntohs(*(uint16_t*)(packet.data));

        if (opcode == 5) {
            fprintf(stderr, "tftp: server error: %s\n", packet.data + 4);
            ERROR;
        } else if (opcode != 4) {
            fprintf(stderr, "tftp: unexpected opcode %d during send\n", opcode);
            ERROR;
        }

        uint16_t block_num_ack = ntohs(*(uint16_t*)(packet.data + 2));
        if (block_num_ack != block_num) {
            fprintf(stderr, "tftp: received ACK for block %d instead of %d\n", block_num_ack, block_num);
            ERROR;
        }

        if (display_mode)
            printf(ANSI_UPCLEAR "%s: %d bytes (block %d)\n", fileName, block_num * 512, block_num);

        block_num++;
    }

    fclose(file);
    return 0;
}

/*********************************
 *                              *
 *    Command Line Interface    *
 *                              *
*********************************/

in_port_t parse_port(char lettre) {
    int port = carp_get_int(lettre);
    if (port == -1)
        return TFTP_PORT;
    if (port < 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %d\n", port);
        exit(1);
    }
    return (in_port_t) port;
}

int main(int argc, char **argv) {
    carp_init("[-l <local_port>] [-p <remote_port>] [-q] [-n] <server_ip> <filename>", CARP_FMIN(2) | 2);

    carp_register('l', CARP_NEXT_INT, "local port (default: 69)");
    carp_register('p', CARP_NEXT_INT, "remote port (default: 69)");
    carp_register('q', CARP_STANDARD, "quiet, no transfer information");
    carp_register('s', CARP_STANDARD, "send file instead of receiving it");

    if (carp_parse(argc, argv)) {
        return 1;
    }

    src_port  = parse_port('l');
    dest_port = parse_port('p');    

    const char **args = carp_get_files();

    display_mode = !(carp_isset('q') || !isatty(STDOUT_FILENO));

    if (inet_aton(args[0], (void *) &server_ip) == 0) {
        fprintf(stderr, "tftp: invalid server IP address: %s\n", args[0]);
        return 1;
    }

    if (pni_init() != 0) {
        fprintf(stderr, "Failed to initialize PNI\n");
        return 1;
    }

    if (carp_isset('s')) {
        tftp_send_file(args[1]);
    } else {
        tftp_get_file(args[1]);
    }

    pni_exit();
    return 0;
}
