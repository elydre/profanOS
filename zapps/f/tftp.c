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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <profan/pni.h>
#include <profan/carp.h>
#include <unistd.h>

#define TFTP_PORT 69
#define ANSI_UPCLEAR "\e[1A\e[2K"

in_port_t src_port, dest_port;
in_addr_t server_ip;
int display_mode = 0;

#define DEBUG(x) do { if (x != 0) { \
    fprintf(stderr, "Error at line %d: %s\n", __LINE__, x); \
} } while (0)

void send_ack(in_port_t dest, uint16_t block_num) {
    uint8_t packet[4];
    uint16_t tmp;

    tmp = htons(4); // ACK opcode
    memcpy(packet, &tmp, 2);

    tmp = htons(block_num); // Block number
    memcpy(packet + 2, &tmp, 2);

    DEBUG(pni_send(src_port, dest, packet, sizeof(packet), (uint8_t *) server_ip));
}

void send_rrq(const char *filename) {
    uint8_t packet[516];
    uint16_t opcode = htons(1);
    int len = strlen(filename);

    memcpy(packet, &opcode, 2);
    strlcpy((char *) packet + 2, filename, 500);
    strcpy((char *) packet + 3 + len, "octet");

    DEBUG(pni_send(src_port, dest_port, packet, len + 9, (uint8_t *) server_ip));
}

/*
void send_data(uint8_t *addr, uint16_t block_num, const char *data, size_t len) {
    char packet[516];
    uint16_t opcode = htons(3);
    memcpy(packet, &opcode, 2);
    uint16_t blocknum = block_num;
    memcpy(packet + 2, &blocknum, 2);
    memcpy(packet + 4, data, len);
    DEBUG(pni_send(src_port, dest_port, (uint8_t *) packet, 4 + len, addr));
    printf("Sent DATA packet for block: %d, length: %zu\n", block_num, len);
}

void sendWRQ(uint8_t *server, const char *filename) {
    char packet[516];
    int position = 0;
    uint16_t opcode = htons(2);
    memcpy(packet + position, &opcode, 2);
    position += 2;

    strcpy(packet + position, filename);
    position += strlen(filename) + 1;
    strcpy(packet + position, MODE);
    position += strlen(MODE) + 1;
    DEBUG(pni_send(src_port, dest_port, (uint8_t *) packet, position, server));
    printf("Sent WRQ for file: %s\n", filename);
}

void PutFile(uint8_t *server_addr, const char *fileName) {
    sendWRQ(server_addr, fileName);

    int packet.len;
    uint8_t *packet.data = pni_recv(&packet.len, 0);
    printf("Received packet of length: %d\n", packet.len);

    if (packet.len < 4) {
        // fprintf(stderr, "Invalid acknowledgment packet\n");
        return;
    }

    uint16_t opcode = ntohs(*(uint16_t*)(packet.data));
    uint16_t block_num_ack = ntohs(*(uint16_t*)(packet.data + 2));

    if (opcode == 5) {
        fprintf(stderr, "Server Error: %s\n", packet.data + 4);
        return;
    } else if (opcode != 4 || block_num_ack != 0) {
        fprintf(stderr, "Unexpected opcode or block number (opcode: %d, block_num: %d)\n", opcode, block_num_ack);
        return;
    }

    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    uint16_t block_num = 1;
    while (1) {
        char data[512];
        fseek(file, (block_num - 1) * 512, SEEK_SET);
        size_t read_len = fread(data, 1, 512, file);

        send_data(server_addr, block_num, data, read_len);
        printf("Sent DATA packet for block: %d\n", block_num);

        packet.data = pni_recv(&packet.len, 0);

        printf("Received ACK packet of length: %d\n", packet.len);
        if (packet.len < 4) {
            // fprintf(stderr, "Invalid ACK packet\n");
            break;
        }

        opcode = ntohs(*(uint16_t*)(packet.data));
        block_num_ack = ntohs(*(uint16_t*)(packet.data + 2));

        if (opcode == 5) {
            fprintf(stderr, "Server Error: %s\n", packet.data + 4);
            break;
        } else if (opcode != 4 || block_num_ack != block_num) {
            fprintf(stderr, "Unexpected ACK (opcode: %d, block_num: %d)\n", opcode, block_num_ack);
            break;
        }

        printf("Received ACK for block: %d\n", block_num);
        block_num++;

        if (read_len < 512) {
            printf("End of transfer\n");
            break;
        }
    }

    fclose(file);
    fflush(stdout);
}
*/

void getFile(const char *filename) {
    send_rrq(filename);

    pni_packet_t packet;
    packet.data = NULL;

    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    if (display_mode)
        printf("%s: starting transfer...\n", filename);

    uint16_t expected_block = 1;
    int retries = 0;

    while (1) {
        free(packet.data);

        packet = pni_recv(PNI_RECV_HANGTIME, 1000);

        if (packet.len < 4) {
            fprintf(stderr, "tftp: received packet too short\n");
            break;
        }

        uint16_t opcode = ntohs(*(uint16_t *)(packet.data));
        
        if (opcode == 5) {
            fprintf(stderr, "tftp: server error: %s\n", packet.data + 4);
            break;
        } else if (opcode != 3) {
            fprintf(stderr, "tftp: unexpected opcode %d during receive\n", opcode);
            break;
        }

        uint16_t blocknum = ntohs(*(uint16_t *)(packet.data + 2));

        if (blocknum != expected_block) {
            if (retries >= 10) {
                fprintf(stderr, "tftp: too many retries, aborting transfer\n");
                break;
            }

            if (display_mode)
                printf(ANSI_UPCLEAR "%s: wrong block number, retry...\n", filename);
            
            // Resend ACK for the expected block number
            if (blocknum < expected_block)
                send_ack(dest_port, expected_block);
            
            retries++;
            continue;
        }

        retries = 0;

        size_t data_len = packet.len - 4;
        fwrite(packet.data + 4, 1, data_len, file);

        if (display_mode)
            printf(ANSI_UPCLEAR"%s: %d bytes (block %d)\n", filename, blocknum * 512, blocknum);

        send_ack(packet.src_port, blocknum);
        expected_block++;

        if (data_len < 512) {
            printf(ANSI_UPCLEAR"%s: transfer complete, %d bytes\n", filename, ftell(file));
            break;
        }
    }

    free(packet.data);
    fclose(file);
}

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
        printf("Sending file is not implemented yet.\n");
    } else {
        getFile(args[1]);
    }

    pni_exit();
    return 0;
}
