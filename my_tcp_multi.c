#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 4242 // le port de notre serveur

int create_server_socket(void);

void accept_new_connection(int listener_socket, fd_set *all_sockets, int *fd_max);

void read_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket);

int main(void)
{
	int server_socket;
	int status;
	int i;
	fd_set all_sockets; // Ensemble de toutes les sockets du serveur
	fd_set read_fds;
	int fd_max;
	struct timeval timer;

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
	FD_SET(server_socket, &all_sockets); // Ajout de la socket principale à
	fd_max = server_socket; // Le descripteur le plus grand est forcément celui de

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
			printf("[Server] Waiting...\n");
			continue;
		}

		i = 0;
		while (i <= fd_max)
		{
			if (FD_ISSET(i, &read_fds) != 1)
			{
				i++;
				continue ;
			}
			printf("[%d] Ready for I/O operation\n", i);
			if (i == server_socket)
				accept_new_connection(server_socket, &all_sockets, &fd_max);
			else
				read_data_from_socket(i, &all_sockets, fd_max, server_socket);
			i++;
		}
	}
	return (0);
}

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

void accept_new_connection(int server_socket, fd_set *all_sockets, int *fd_max)
{
	int client_fd;
	char msg_to_send[BUFSIZ];
	int status;

	client_fd = accept(server_socket, NULL, NULL);
	if (client_fd == -1)
	{
	printf("[Server] Accept error: %s\n", strerror(errno));
	return;
	}
	FD_SET(client_fd, all_sockets); // Ajoute la socket client à l'ensemble
	if (client_fd > *fd_max)
	*fd_max = client_fd; // Met à jour la plus grande socket
	printf("[Server] Accepted new connection on client socket %d.\n", client_fd);
	memset(&msg_to_send, '\0', sizeof msg_to_send);
	sprintf(msg_to_send, "Welcome. You are client fd [%d]\n", client_fd);
	status = send(client_fd, msg_to_send, strlen(msg_to_send), 0);
	if (status == -1)
	printf("[Server] Send error to client %d: %s\n", client_fd, strerror(errno));
}

void read_data_from_socket(int socket, fd_set *all_sockets, int fd_max, int server_socket)
{
	char buffer[BUFSIZ];
	char msg_to_send[BUFSIZ];
	int bytes_read;
	int status;

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
	}
	else
	printf("[%d] Got message: %s", socket, buffer);
}