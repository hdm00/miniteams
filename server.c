#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

int *buffer;

void startup()
{
	printf("Miniteams starting...\n");
	printf("My PID is %i\n", getpid());
	printf("Waiting for new messages\n");

	//allocate max message length size
	buffer = malloc(1010 * sizeof *buffer);
}

void parser(int len)
{
	//Initialize printable message buffer with message length + 1 to allow for null character 
	char message[len + 1];

	//Copy buffer into printable message buffer
	for (int i = 0; i < len; i++)
	{
		message[i] = *(buffer + i);
	}

	//add null character at the end of the message
	message[len] = '\0';

	// Get current time
    time_t current_time;
    time(&current_time);

    // Format time 
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&current_time));

    printf("%s : ", timestamp);

	printf("%s \n", message);

	// Open and write log file in append mode
    int logFile = open("conversations.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (logFile == -1)
    {
        perror("Could not open log file");
        exit(1);
    }
    write(logFile, timestamp, 20);
    write(logFile, " : ", 3);
	write(logFile, message, len);
	write(logFile, "\n", 1);
}

void sig_handler(int sig, siginfo_t* info, void* vp)
{
	int character = info->si_value.sival_int;

	*buffer = character;


	if (*buffer == '\0')			//end of message
	{
		int len = *(buffer - 1);	//get length of message
		buffer -= len + 1;			//reset buffer pointer for future messages
		parser(len);
		
	}	
	else							
		buffer++;					//set buffer pointer for the next character
}

int main(int argc, char const *argv[])
{
	startup();
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sig_handler;

	sigaction(SIGUSR1, &sa, NULL);

	while(1)
	{
		pause();
	}

	free(buffer);
	return 0;
}
