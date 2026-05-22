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

    printf("Connecté à google.com:80\n");

    // Fermeture immédiate
    close(sock);

    printf("Connexion fermée\n");

    return 0;
}
