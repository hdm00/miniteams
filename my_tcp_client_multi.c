#include<stdio.h>
#include<stdlib.h>
#include<netdb.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>

#define BUFFER_SIZE 256

struct thread_args {
    int socket_desc;
};

void *receive_messages(void *args) {
    struct thread_args *targs = (struct thread_args *) args;
    char buffer[BUFFER_SIZE];
    int n;

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        n = read(targs->socket_desc, buffer, BUFFER_SIZE - 1);
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        } else if (n == 0) {
            printf("Connection closed by server.\n");
            exit(0);
        }
        printf("\nNouveau message: %s", buffer);
    }
}

int main(int argc, char **argv) {
    int socket_desc;
    int portno;
    struct sockaddr_in server_addr;
    struct hostent *server;
    pthread_t tid;
    struct thread_args targs;

    char buffer[BUFFER_SIZE];
    int n;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);

    // création socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portno);

    // connexion au serveur
    if (connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    // Lecture de la réponse du serveur
    bzero(buffer, BUFFER_SIZE);
    n = read(socket_desc, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("\033[2J");
    printf("[SERVER]: %s\n", buffer);

    // Lancement du thread pour la réception des messages
    targs.socket_desc = socket_desc;
    pthread_create(&tid, NULL, receive_messages, (void *) &targs);

    // Boucle principale pour l'envoi de messages
    while (1) {
        printf("Please enter a message: ");
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);

        // envoie du message
        n = write(socket_desc, buffer, strlen(buffer));
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
    }

    return 0;
}
