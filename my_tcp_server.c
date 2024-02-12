#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // inet_addr
#include<string.h>

int main(int argc, char *argv[]) 
{
    printf("Début main\n");
    // variables
    int socket_desc, new_socket, c;
    // creation d'un objet de type sockaddr_in
    struct sockaddr_in server, client;

    // Vérification du nombre d'arguments
    if (argc < 2)
    {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    // Obtention de l'adresse IP et du port du serveur à partir des arguments de la ligne de commande
    int port_srv = atoi(argv[1]);

    // creation du socket -> socket_desc
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) 
    {
        printf("Could not create socket");
        return 1;
    }

    // initialisation des membres de l'objet server
    printf("Port: %d\n", port_srv);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons (port_srv);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("\nbind failed\n");
    }
    puts("\nbinding done\n");

    //ecoute
    listen(socket_desc, 3);

    //Accept incoming connection
    puts("Waiting for incoming connections... \n");
    c = sizeof(struct sockaddr_in);
    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (new_socket < 0);
    {
        perror("Accept failed\n");
    }

    puts("Connection accepted\n")

    return 0;
}
