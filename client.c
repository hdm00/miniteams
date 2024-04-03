#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 256

struct thread_args {
    int socket_desc; // Structure pour passer les arguments au thread
};

// Fonction exécutée par le thread pour recevoir les messages du serveur
void *receive_messages(void *args) {
    struct thread_args *targs = (struct thread_args *) args;
    char buffer[BUFFER_SIZE];
    int n;

    // Boucle pour recevoir en continu les messages du serveur
    while (1) {
        bzero(buffer, BUFFER_SIZE); // Efface le contenu du tampon
        n = read(targs->socket_desc, buffer, BUFFER_SIZE - 1); // Lecture du message du serveur
        if (n < 0) {
            perror("ERROR reading from socket"); // Affiche une erreur si la lecture échoue
            exit(1);
        } else if (n == 0) {
            printf("Connection closed by server.\n"); // Indique que le serveur a fermé la connexion
            exit(0);
        }
        printf("\n%s", buffer); // Affiche le message reçu du serveur
        printf("\nPlease enter a message: "); // Invite l'utilisateur à saisir un message
        fflush(stdout); // Assure l'affichage immédiat, évite que les boucles empiètent l'une sur l'autre
    }
}

int main(int argc, char **argv) {
    int socket_desc; // Descripteur de socket
    int portno; // Numéro de port
    struct sockaddr_in server_addr; // Structure pour les informations du serveur
    struct hostent *server; // Structure pour l'hôte
    pthread_t tid; // Identifiant du thread
    struct thread_args targs; // Structure pour les arguments du thread

    char buffer[BUFFER_SIZE]; // Tampon pour stocker les messages
    int n;

    // Vérifie les arguments de la ligne de commande
    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]); // Convertit le numéro de port en entier

    // Crée une socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        perror("ERROR opening socket"); // Affiche une erreur en cas d'échec de création de la socket
        exit(1);
    }

    server = gethostbyname(argv[1]); // Récupère les informations sur le serveur

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n"); // Affiche une erreur si l'hôte est introuvable
        exit(0);
    }

    // Initialise la structure de l'adresse du serveur
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portno);

    // Établit la connexion avec le serveur
    if (connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR connecting"); // Affiche une erreur en cas d'échec de connexion au serveur
        exit(1);
    }

    // Envoie le pseudonyme au serveur
    n = write(socket_desc, argv[3], strlen(argv[3]));
    if (n < 0) {
        perror("ERROR writing to socket"); // Affiche une erreur en cas d'échec d'envoi du pseudonyme
        exit(1);
    }

    // Lit la réponse du serveur
    bzero(buffer, BUFFER_SIZE);
    n = read(socket_desc, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
        perror("ERROR reading from socket"); // Affiche une erreur en cas d'échec de lecture de la réponse du serveur
        exit(1);
    }

    printf("\033[2J"); // Efface l'écran
    printf("[SERVER]: %s\n", buffer); // Affiche la réponse du serveur

    // Lancement du thread pour la réception des messages
    targs.socket_desc = socket_desc;
    pthread_create(&tid, NULL, receive_messages, (void *) &targs);

    // Boucle principale pour l'envoi de messages
    while (1) {
        printf("Please enter a message: ");
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin); // Lecture du message à envoyer depuis l'entrée standard

        // Envoie du message
        n = write(socket_desc, buffer, strlen(buffer));
        if (n < 0) {
            perror("ERROR writing to socket"); // Affiche une erreur en cas d'échec d'envoi du message
            exit(1);
        }
    }

    return 0;
}
