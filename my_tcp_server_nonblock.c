#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // inet_addr
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>

int main(int argc, char *argv[]) 
{
    // variables
    int socket_desc, new_socket, c;
    // creation d'un objet de type sockaddr_in
    struct sockaddr_in server, client;
    char client_message[2000];

    // Vérification du nombre d'arguments
    if (argc < 2)
    {
        printf("Usage: %s <server_port>\n", argv[0]);
        return 1;
    }

    // Obtention du port du serveur à partir des arguments de la ligne de commande
    int port_srv = atoi(argv[1]);

    // creation du socket -> socket_desc
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(socket_desc, F_SETFL, O_NONBLOCK);
    if (socket_desc == -1) 
    {
        printf("Could not create socket");
        return 1;
    }

    // initialisation des membres de l'objet server
    printf("Port: %d\n", port_srv);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_srv);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("\nbind failed\n");
        return 1;
    }
    puts("\nbinding done\n");

    //ecoute
    listen(socket_desc, 3);

    //Accept incoming connection
    puts("Waiting for client message... \n");
    c = sizeof(struct sockaddr_in);

    // Boucle pour accepter plusieurs connexions
    while (1)
    {
        new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (new_socket < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // Ressource temporairement non disponible, continuer à attendre
                printf("je suis en train d'attendre\n");
                sleep(2);
                continue;
            }
            perror("Accept failed\n");
            return 1;
        }

        //Receive a message from client
        int recv_result;
        do {
            recv_result = recv(new_socket, client_message, 2000, 0);
            if (recv_result > 0) {
                printf("Message from client %s:%d --> %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), client_message);
            } else if (recv_result < 0 && errno != EWOULDBLOCK) {
                perror("Receive failed\n");
                return 1;
            }
            // Sleeping for 2 seconds...
            sleep(2);
        } while (recv_result == -1);

        // Close the socket after receiving the message
        close(new_socket);
    }

    return 0;
}
