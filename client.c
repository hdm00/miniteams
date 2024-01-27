#include <stdio.h> /* librairie de base */
#include <unistd.h> /*Integration pour gestion pause et exit */
#include <signal.h> /*Integration pour gestion des signals */
#include <stdlib.h> /*Integration fonction exit */
#include <string.h> /* Manipulation des chaines de caractères et de la mémoire */
#include <ctype.h> /* Repérage de type de caractères avec les préfixes is-fonction */


void traitement_msg(int *message_propre, char *message_brut, int taille_msg) /*fonction permettant de rendre traitable le message reçu */
{
    int iterator = 0;
    while (iterator < taille_msg)
    {
        message_propre[iterator] = message_brut[iterator];
        iterator++;
    }

    message_propre[taille_msg] = taille_msg;
    message_propre[taille_msg + 1] = '\0'; /* On ajoute le \0 en fin de message pour le rendre lisible correctement */

}

int main(int argc, char const *argv[]) /* Creation de la fonction main */
{
	if (argc != 3) /* On vérifie qu'il y ai bien 3 arguments en arrivée */
    {
        printf("Merci de creer votre message sous la forme PID destination + \"message\" \n");

        return 1;
    }

	int len = strlen(argv[2]); /* On intitalise la variable len, prenant la longueur du message en argument. */

	char message_brut[len]; /* On intialise la variable message_brut, qui permet d'allouer la mémoire nécessaire au traitement du message */

	int maxlen = 127; /*On initialise la valeur de la longueur max. */
	
	if (len > maxlen) /* On vérifie la taille du message en entrée */
	{
    	perror("Le message est trop long, il doit faire moins de 128 caractères\n");
    }
    else
    {
    	strcpy(message_brut, argv[2]); /* On copie le message passé en argument du client dans la variable crée précedemment */

		int new_message[len + 2]; /* On crée une nouvelle variable, contenant l'espace mémoire disponible pour le message et le \n. */
	
		traitement_msg(new_message, message_brut, len); /* On passe le message entré en argument dans la fonction le rendant "propre" */
	
		int iterator = 0; 
		int pid = atoi(argv[1]); /* On transforme le PID passé en argument en entier */
		
		while (iterator < len + 2) /* On initialise la boucle qui parcours chaque caractère du message propre. */
		{

			union sigval value; /* On utilise la technique d'union de la librairie signal.h, afin de définir une union entre value, et la valeur entière qui lui est associée dans le signal. */

			if (!isascii(new_message[iterator])) /* On utilise la fonction isascii de la librairie ctype.h, afin de vérifier si les caractères font parti de l'ASCII. */
			{
				printf("Merci de ne renseigner que des caractères ASCII.\n");
				break;
			}

			value.sival_int = new_message[iterator]; /*Après avoir vérifié que le caractère est ASCII, on l'attribue a la valeur du signal */
			iterator++;

			if(sigqueue(pid, SIGUSR1, value) != 0)	/* On envoie ensuite le caractère sous forme de valeur du signal SIGUSR1 au pid renseigné, en prennant en compte la gestion d'erreur */
			{
				perror("Le message n'a pas été envoyé, voici le message d'erreur:\n");
				break;
			}
			sleep(0.01); /* On  met en place une micropause pour laisser le temps au serveur de recevoir chaque signal à la suite. */

		}
	}

	return 0;
}
