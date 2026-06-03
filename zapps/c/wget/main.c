/*****************************************************************************\
|   === main.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>
#include "./picohttpparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "picohttpparser.h"

#define BUF_SIZE 8192
#define MAX_HEADERS 32

void parse_http_response(int sock)
{
    char buf[BUF_SIZE];
    ssize_t rret;
    size_t buflen = 0;
    int pret;

    int minor_version;
    int status;
    const char *msg;
    size_t msg_len;

    struct phr_header headers[MAX_HEADERS];
    size_t num_headers = MAX_HEADERS;

    while (1) {
        rret = recv(sock, buf + buflen, sizeof(buf) - buflen, 0);

        if (rret <= 0) {
            perror("recv");
            return;
        }

        buflen += rret;

        pret = phr_parse_response(
            buf,
            buflen,
            &minor_version,
            &status,
            &msg,
            &msg_len,
            headers,
            &num_headers,
            0
        );

        if (pret > 0) {
            // Parsing OK

            printf("HTTP/1.%d %d %.*s\n",
                   minor_version,
                   status,
                   (int)msg_len,
                   msg);

            for (size_t i = 0; i < num_headers; i++) {
                printf("%.*s: %.*s\n",
                       (int)headers[i].name_len,
                       headers[i].name,
                       (int)headers[i].value_len,
                       headers[i].value);
            }

            printf("\n");

            // body
            size_t body_len = buflen - pret;
            char *body = buf + pret;

            printf("BODY (%zu bytes):\n", body_len);
            fwrite(body, 1, body_len, stdout);
            printf("\n");

            return;
        }
        else if (pret == -1) {
            fprintf(stderr, "Parse error\n");
            return;
        }

        // pret == -2 => incomplete response
        if (buflen == sizeof(buf)) {
            fprintf(stderr, "Buffer too small\n");
            return;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2)
        return 1;
    struct hostent *host;
    struct sockaddr_in addr;
    int sock;

    host = gethostbyname(argv[1]);
    if (host == NULL) {
        perror("gethostbyname");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    memcpy(&addr.sin_addr, host->h_addr_list[0], 4);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    char request[4096];
    sprintf(request,
    "GET / HTTP/1.1\r\n"
    "Host: %s\r\n"
    "\r\n"
    "\r\n", argv[1]);
    send(sock, request, strlen(request), 0);
    shutdown(sock, SHUT_WR);

    //parse_http_response(sock);
    while (1) {
        char buff[4096 + 1];
        int ret = recv(sock, buff, 4096, 0);
        if (ret <= 0)
            break;
        buff[ret] = '\0';
        printf("%s\n", buff);
    }

    close(sock);

    return 0;
}
