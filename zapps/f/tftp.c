// @LINK: libpf, libpni

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <profan/pni.h>
#include <profan/carp.h>
#include <unistd.h>

#define TFTP_PORT 25565
#define MODE "octet"
#define MAXL 100

uint16_t src_port, dest_port;

#define DEBUG(x) do { if (x != 0) { \
    fprintf(stderr, "Error at line %d: %s\n", __LINE__, x); \
} } while (0)


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

    int recv_len;
    uint8_t *buffer = pni_recv(&recv_len, 0);
    printf("Received packet of length: %d\n", recv_len);

    if (recv_len < 4) {
        // fprintf(stderr, "Invalid acknowledgment packet\n");
        return;
    }

    uint16_t opcode = ntohs(*(uint16_t*)(buffer));
    uint16_t block_num_ack = ntohs(*(uint16_t*)(buffer + 2));

    if (opcode == 5) {
        fprintf(stderr, "Server Error: %s\n", buffer + 4);
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

        buffer = pni_recv(&recv_len, 0);

        printf("Received ACK packet of length: %d\n", recv_len);
        if (recv_len < 4) {
            // fprintf(stderr, "Invalid ACK packet\n");
            break;
        }

        opcode = ntohs(*(uint16_t*)(buffer));
        block_num_ack = ntohs(*(uint16_t*)(buffer + 2));

        if (opcode == 5) {
            fprintf(stderr, "Server Error: %s\n", buffer + 4);
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

void send_ack(uint8_t *serv_addr, uint16_t block_num) {
    uint8_t packet[4];
    uint16_t opcode = htons(4);
    uint16_t block_len = htons(block_num);
    memcpy(packet, &opcode, 2);
    memcpy(packet + 2, &block_len, 2);
    DEBUG(pni_send(src_port, pni_dest_ip(), packet, sizeof(packet), serv_addr));
    printf("Sent ACK for block: %d\n", block_num);
}

void sendRRQ(uint8_t *server, const char *filename) {
    uint8_t packet[516];
    uint16_t opcode = htons(1);
    memcpy(packet, &opcode, 2);
    strcpy((char *) packet + 2, filename);
    strcpy((char *) packet + 2 + strlen(filename) + 1, MODE);
    size_t len = 4 + strlen(filename) + strlen(MODE);
    DEBUG(pni_send(src_port, dest_port, packet, len, server));
    printf("Sent RRQ for file: %s\n", filename);
}

void getFile(uint8_t *server, const char *filename) {
    sendRRQ(server, filename);

    int recv_len;

    uint8_t *buffer;
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    uint16_t expected_block = 1;
    while (1) {
        // ssize_t recv_len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&transfer_addr, &addr_len);
        buffer = pni_recv(&recv_len, PNI_RECV_HANGTIME, 6000);
        printf("Received packet of length: %d\n", recv_len);

        if (recv_len < 4) {
            // printf("Received packet with invalid length\n");
            break;
        }

        uint16_t opcode = ntohs(*(uint16_t *)(buffer));
        if (opcode == 3) {
            uint16_t blocknum = ntohs(*(uint16_t *)(buffer + 2));
            printf("-> DATA packet for block: %d\n", blocknum);

            if (blocknum == expected_block) {
                size_t data_len = recv_len - 4;
                fwrite(buffer + 4, 1, data_len, file);
                printf("-> Written %u bytes to file\n", data_len);
                send_ack(server, blocknum);

                expected_block++;
                if (data_len < 512) {
                    printf("End of file transfer\n");
                    break;
                }
            } else {
                printf("Unexpected block number: %d (expected: %d)\n", blocknum, expected_block);
            }
        } else if (opcode == 5) {
            fprintf(stderr, "-> Error: %s\n", buffer + 4);
            break;
        } else {
            printf("Unexpected opcode: %d\n", opcode);
            break;
        }
    }

    fclose(file);
}

int str_to_ip(const char *src, unsigned char *dst) {
    int saw_digit, octets, ch;
    unsigned char tmp[4], *tp;
    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while (*src) {
        ch = *src++;
        if (ch >= '0' && ch <= '9') {
            unsigned int new = *tp * 10 + (ch - '0');
            if (saw_digit && *tp == 0)
                return 1;
            if (new > 255)
                return 1;
            *tp = new;
            if (!saw_digit) {
                if (++octets > 4)
                    return 1;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit) {
            if (octets == 4)
                return 1;
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return 1;
    }
    if (octets < 4)
        return 1;
    memcpy(dst, tmp, 4);
    return 0;
}

int main(int argc, char **argv) {
    carp_init("[-l <local_port>] [-r <remote_port>] [-s] <server_ip> <filename>", CARP_FMIN(2) | 2);

    carp_register('l', CARP_NEXT_INT, "local port (default: 69)");
    carp_register('r', CARP_NEXT_INT, "remote port (default: 69)");
    carp_register('s', CARP_STANDARD, "send file instead of receiving it");

    if (carp_parse(argc, argv)) {
        return 1;
    }

    int tmp;

    tmp = carp_get_int('l');
    if (tmp != -1) {
        if (tmp < 0 || tmp > 65535) {
            fprintf(stderr, "Invalid local port: %d\n", tmp);
            return 1;
        }
        src_port = tmp;
    } else {
        src_port = TFTP_PORT;
    }

    tmp = carp_get_int('r');
    if (tmp != -1) {
        if (tmp < 0 || tmp > 65535) {
            fprintf(stderr, "Invalid remote port: %d\n", tmp);
            return 1;
        }
        dest_port = tmp;
    } else {
        dest_port = TFTP_PORT;
    }

    const char **args = carp_get_files();

    uint8_t server_ip[4];

    if (str_to_ip(args[0], server_ip) != 0) {
        fprintf(stderr, "Invalid server IP address: %s\n", args[0]);
        return 1;
    }

    printf("Using server IP: %d.%d.%d.%d\n", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);

    if (pni_init() != 0) {
        fprintf(stderr, "Failed to initialize PNI\n");
        return 1;
    }

    if (carp_isset('s')) {
        // PutFile(server_ip, args[1]);
        printf("Sending file is not implemented yet.\n");
    } else {
        getFile(server_ip, args[1]);
    }

    pni_exit();
    return 0;
}
