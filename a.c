#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    const char *server_ip = "45.79.112.203";
    uint16_t server_port = 4242;
    int sock;
    struct sockaddr_in serv_addr;

    // 1️⃣ Crée le socket TCP
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }

    // 2️⃣ Connexion au serveur
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    printf("Connecté au serveur %s:%d\n", server_ip, server_port);

    // 3️⃣ Envoi d'un message
    const char *msg_send = "salut\n";
    if (write(sock, msg_send, strlen(msg_send)) < 0) {
        perror("write");
        close(sock);
        return 1;
    }

    // 4️⃣ Lecture de la réponse
    char msg_recv[100] = {0};
    int n = read(sock, msg_recv, sizeof(msg_recv)-1);
    if (n < 0) {
        perror("read");
    } else if (n == 0) {
        printf("Serveur fermé la connexion\n");
    } else {
        printf("Réponse du serveur (%d octets) : %s\n", n, msg_recv);
    }

    // 5️⃣ Fermeture propre
    close(sock);
    printf("Connexion fermée\n");

    return 0;
}

