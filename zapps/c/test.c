/*****************************************************************************\
|   === test.c : 2026 ===                                                     |
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
int main(void)
{
    struct hostent *host;
    struct sockaddr_in addr;
    int sock;

    // Résolution DNS
    host = gethostbyname("asqel.ddns.net");
    if (host == NULL) {
        perror("gethostbyname");
        return 1;
    }

    // Création du socket TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    // Copie de l'IP résolue
    memcpy(&addr.sin_addr, host->h_addr_list[0], 4);

    // Connexion
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    char *request =
    "GET / HTTP/1.1\r\n"
    "Host: asqel.ddns.net\r\n"
    "\r\n"
    "\r\n";
    send(sock, request, strlen(request), 0);
    while (1) {
        char buf[512];
        int ret = recv(sock, buf, 511, 0);
        if (ret <= 0)
            break;
        buf[ret] = '\0';
        printf("%s\n", buf);
    }

    printf("Connecté à google.com:80\n");

    // Fermeture immédiate
    close(sock);

    printf("Connexion fermée\n");

    return 0;
}
