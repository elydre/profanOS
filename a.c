#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Créer le socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Réutiliser l’adresse rapidement après un crash
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Préparer l’adresse
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // écoute sur toutes les interfaces
    address.sin_port = htons(PORT);

    // Lier le socket à l’adresse
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Écouter
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Serveur TCP démarré sur port %d...\n", PORT);

    while (1) {
        // Accepter une connexion
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client connecté !\n");

        // Envoyer un message au client
        char *msg = "Hello depuis le serveur TCP sur l'hôte !\n";
        send(client_fd, msg, strlen(msg), 0);

        close(client_fd);
    }

    return 0;
}
