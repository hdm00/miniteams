#include <stdio.h> /* librairie de base */
#include <unistd.h> /*Integration pour gestion pause et exit */
#include <signal.h> /*Integration pour gestion des signals */
#include <stdlib.h> /*Integration fonction exit */
#include <string.h> /* Manipulation des chaines de caractères et de la mémoire */
#include <ctype.h> /* Repérage de type de caractères avec les préfixes is-fonction */

void traitement_msg(int *message_propre, char *message_brut, int taille_msg) /*fonction permettant de rendre traitable le message reçu */
{
    for (int i = 0; i < taille_msg;)
    {
        message_propre[i] = message_brut[i];
        i++;
    }
    
    /* On se positionne au dernier rang du message, puis on ajoute le '\0' pour detection de sa fin */
    message_propre[taille_msg] = taille_msg;
    message_propre[taille_msg + 1] = '\0';

}

int main(int argc, char const *argv[]) /* Creation de la fonction main */
{
	if (argc != 3) /* On vérifie qu'il y ai bien 3 arguments en arrivée */
    {
        printf("""Merci de creer votre message sous la forme PID destination + 'message' \n""");

        return 1;
    }

	int len = strlen(argv[2]);						//message length

	char message[len];								//initialize buffer for message argument
	
	if (len > 1000)
	{
    	fprintf(stderr, "message is too long!\n");
    	printf("HERE\n");
    }
    else
    {
    	strcpy(message, argv[2]);					//copy argument into buffer

		int new_message[len + 2];					//initialize integer buffer in order to send message
	
		traitement_msg(new_message, message, len);
	
		for (int i = 0; i < len + 2; i++)			//send message
		{
			union sigval value;						//initialize structure containing character value
			
			int pid = atoi(argv[1]);				//get pid passed as an argument

			if (!isascii(new_message[i]))			//check if character is ASCII or not
			{
				perror("one of the character after is not an ASCII character\n");
				break;
			}

			value.sival_int = new_message[i];		

			if(sigqueue(pid, SIGUSR1, value) != 0)	//send character ASCII value
			{
				perror("message could not be send\n");
				break;
			}
			sleep(0.01);							//wait to allow server to correctly receive the signal
		}
	}

	return 0;
}
