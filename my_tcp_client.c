#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // inet_addr
#include<string.h>

int main(int argc, char *argv[]) 
{
    printf("Début main\n");
    // variables
    int socket_desc;
    // creation d'un objet de type sockaddr_in
    struct sockaddr_in server;

    // Vérification du nombre d'arguments
    if (argc < 4) 
    {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    // Obtention de l'adresse IP et du port du serveur à partir des arguments de la ligne de commande
    char *ip_srv = argv[1];
    int port_srv = atoi(argv[2]);
    char *message_entre = argv[3];

    // creation du socket -> socket_desc
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) 
    {
        printf("Could not create socket");
        return 1;
    }

    // initialisation des membres de l'objet server
    printf("Server IP: %s\n", ip_srv);
    printf("Port: %d\n", port_srv);
    server.sin_addr.s_addr = inet_addr(ip_srv);
    server.sin_family = AF_INET;
    server.sin_port = htons(port_srv);

    // connexion a un serveur distant
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Connect error");
        return 1;
    }

    puts("Connected");

    // envoie de donnees 
    if (send(socket_desc, message_entre, strlen(message_entre), 0) < 0)
    {
        puts("Send failed\n");
        return 1;
    }

    puts("Data Sent\n");
}
