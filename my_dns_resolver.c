#include<stdio.h> //printf
#include<string.h> //strcpy
#include<sys/socket.h>
#include<netdb.h> //hostent
#include<arpa/inet.h>

int main(int argc, char **argv)
{
    if(argc != 2){
        printf("Merci de renseigner le nom de domaine a la suite du nom du programme.\n");
        return 0;
    }
    char *hostname = argv[1];
    printf("hostname : %s\n", hostname);
    char ip[16];
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr ** )he->h_addr_list;

    strcpy(ip, inet_ntoa(*addr_list[0]));
    
    printf("%s resolved to : %s\n", hostname, ip);

    return 0;
}
