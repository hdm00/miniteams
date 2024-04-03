#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define PORT 4242 // le port de notre serveur
#define MAX_CLIENTS 10
#define NICKNAME_MAX_LENGTH 255
#define MAX_MESSAGE_LENGTH (BUFSIZ + NICKNAME_MAX_LENGTH + 8) // + NICKNAME_MAX_LENGTH pour le pseudonyme et +8 pour "FROM : "
#define BUFFER_SIZE 256
#define MAX_USERS 10

// Structure d'informations sur le client
struct client_info {
    int socket_fd;
    char nickname[NICKNAME_MAX_LENGTH + 1]; // +1 pour le caractère nul
};

char connected_users[MAX_USERS][BUFFER_SIZE];
int num_users = 0; // Déclaration globale de num_users

// Fonction pour créer une socket serveur
int create_server_socket(void);

// Fonction pour accepter une nouvelle connexion client
void accept_new_connection(int listener_socket, fd_set *all_sockets, int *fd_max, struct client_info *clients, int *num_clients);

// Fonction pour lire des données depuis une socket
void read_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket, struct client_info *clients, int num_clients);

// Fonction pour sauvegarder un message dans un fichier log
void saveMessage(const char *message);

// Fonction pour envoyer la liste des utilisateurs connectés à un client spécifié
void send_connected_users(int client_fd, const struct client_info *clients, int num_clients);

// Ajout d'un utilisateur à la liste des utilisateurs connectés
void add_user(const char *nickname);

int main(void)
{
    int server_socket;
    int status;
    int i;
    fd_set all_sockets; // Ensemble de toutes les sockets du serveur
    fd_set read_fds;
    int fd_max;
    struct timeval timer;
    struct client_info clients[MAX_CLIENTS];
    int num_clients = 0; 

    server_socket = create_server_socket();
    if (server_socket == -1)
        exit(-1);

    printf("[Server] Listening on port %d\n", PORT);
    status = listen(server_socket, 10);
    if (status != 0)
    {
        printf("[Server] Listen error: %s\n", strerror(errno));
        exit(-1);
    }

    FD_ZERO(&all_sockets);
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &all_sockets);
    fd_max = server_socket;

    printf("[Server] Set up select fd sets\n");

    while (1) // Boucle principale
    {
        read_fds = all_sockets;
        timer.tv_sec = 2;
        timer.tv_usec = 0;

        status = select(fd_max + 1, &read_fds, NULL, NULL, &timer);
        if (status == -1)
        {
            printf("[Server] Select error: %s\n", strerror(errno));
            exit(-1);
        }
        else if (status == 0)
        {
            continue; // pause serveur 
        }

        i = 0;
        while (i <= fd_max)
        {
            if (FD_ISSET(i, &read_fds) != 1)
            {
                i++;
                continue;
            }
            if (i == server_socket) // Socket prête
                accept_new_connection(server_socket, &all_sockets, &fd_max, clients, &num_clients);
            else
                read_data_from_socket(i, &all_sockets, fd_max, server_socket, clients, num_clients);
            i++;
        }
    }
    return (0);
}

// Crée une socket serveur
int create_server_socket(void)
{
    struct sockaddr_in sa;
    int socket_fd;
    int status;

    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET; // IPv4
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1, localhost
    sa.sin_port = htons(PORT);

    socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        printf("[Server] Socket error: %s\n", strerror(errno));
        return (-1);
    }
    printf("[Server] Created server socket fd: %d\n", socket_fd);

    status = bind(socket_fd, (struct sockaddr *)&sa, sizeof sa);
    if (status != 0)
    {
        printf("[Server] Bind error: %s\n", strerror(errno));
        return (-1);
    }
    printf("[Server] Bound socket to localhost port %d\n", PORT);
    return (socket_fd);
}

// Accepte une nouvelle connexion client
void accept_new_connection(int server_socket, fd_set *all_sockets, int *fd_max, struct client_info *clients, int *num_clients)
{
    int client_fd;
    char msg_to_send[BUFSIZ];
    int status;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
    {
        printf("[Server] Accept error: %s\n", strerror(errno));
        return;
    }

    if (*num_clients >= MAX_CLIENTS)
    {
        printf("[Server] Maximum clients reached. Connection rejected.\n");
        close(client_fd);
        return;
    }

    FD_SET(client_fd, all_sockets);
    clients[*num_clients].socket_fd = client_fd;
    (*num_clients)++;

    if (client_fd > *fd_max)
        *fd_max = client_fd;

    printf("[Server] Accepted new connection on client socket %d.\n", client_fd);

    // Recevoir le pseudonyme du client
    bzero(msg_to_send, BUFSIZ);
    status = recv(client_fd, msg_to_send, BUFSIZ, 0);
    if (status <= 0)
    {
        printf("[Server] Error receiving nickname from client %d.\n", client_fd);
        close(client_fd);
        FD_CLR(client_fd, all_sockets);
        return;
    }

    strncpy(clients[*num_clients - 1].nickname, msg_to_send, strlen(msg_to_send));
    printf("[Server] Client %d is known as %s\n", client_fd, clients[*num_clients - 1].nickname);

    // Ajouter l'utilisateur à la liste des utilisateurs connectés
    add_user(msg_to_send);

    // Envoyer un message de bienvenue au client
    bzero(msg_to_send, BUFSIZ);
    sprintf(msg_to_send, "Welcome, %s. You are client [%d]\n", clients[*num_clients - 1].nickname, client_fd);
    status = send(client_fd, msg_to_send, strlen(msg_to_send), 0);
    if (status == -1)
        printf("[Server] Send error to client %d: %s\n", client_fd, strerror(errno));

    // Envoyer la liste des utilisateurs connectés au nouveau client
    send_connected_users(client_fd, clients, *num_clients);
}

// Lit les données depuis une socket
void read_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket, struct client_info *clients, int num_clients)
{
    char buffer[BUFSIZ];
    int bytes_read;
    int status;
    int i;
    char message_with_nick[MAX_MESSAGE_LENGTH];

    memset(&buffer, '\0', sizeof buffer);
    bytes_read = recv(socket, buffer, BUFSIZ, 0);
    if (bytes_read <= 0)
    {
        if (bytes_read == 0)
            printf("[%d] Client socket closed connection.\n", socket);
        else
            printf("[Server] Recv error: %s\n", strerror(errno));
        close(socket); // Ferme la socket
        FD_CLR(socket, all_sockets); // Enlève la socket de l'ensemble
        // Supprimer le socket fermé de la liste des clients
        for (i = 0; i < num_clients; i++)
        {
            if (clients[i].socket_fd == socket)
            {
                printf("[Server] Client %s (%d) disconnected.\n", clients[i].nickname, socket);
                clients[i].socket_fd = -1;
                break;
            }
        }
        return;
    }
    else
    {
        printf("[%d:%s] %s\n", socket, clients[i].nickname, buffer);
        
        // Créer le message avec le pseudonyme du client expéditeur
        snprintf(message_with_nick, sizeof(message_with_nick), "FROM %s: %s", clients[i].nickname, buffer);

        // Envoyer le message à tous les autres clients
        for (int j = 0; j < num_clients; j++) {
            if (clients[j].socket_fd != -1 && clients[j].socket_fd != server_socket && clients[j].socket_fd != socket) {
                status = send(clients[j].socket_fd, message_with_nick, strlen(message_with_nick), 0);
                if (status == -1)
                    printf("[Server] Send error to client %d: %s\n", clients[j].socket_fd, strerror(errno));
            }
        }

        // Sauvegarder le message dans le fichier log
        saveMessage(message_with_nick);
    }
}

// Sauvegarde un message dans un fichier log
void saveMessage(const char *message) {
    FILE *file = fopen("server_log.txt", "a");
    if (file != NULL) {
        fprintf(file, "%s\n", message);
        fclose(file);
    } else {
        printf("[Server] Error opening log file: %s\n", strerror(errno));
    }
}

// Fonction pour envoyer la liste des utilisateurs connectés à un client spécifié
void send_connected_users(int client_fd, const struct client_info *clients, int num_clients)
{
    char buffer[BUFSIZ];
    bzero(buffer, BUFSIZ);

    strcat(buffer, "\nConnected users:\n");
    for (int i = 0; i < num_clients; i++)
    {
        strcat(buffer, clients[i].nickname);
        strcat(buffer, "\n");
    }

    int status = send(client_fd, buffer, strlen(buffer), 0);
    if (status == -1)
        printf("[Server] Send error to client %d: %s\n", client_fd, strerror(errno));
}

// Ajoute un utilisateur à la liste des utilisateurs connectés
void add_user(const char *nickname)
{
    if (num_users < MAX_USERS)
    {
        strcpy(connected_users[num_users], nickname);
        num_users++;
    }
}
