#include <stdio.h> /* librairie de base */
#include <unistd.h> /*Integration pour gestion pause et exit */
#include <signal.h> /*Integration pour gestion des signals */
#include <stdlib.h> /*Integration fonction exit */
#include <time.h> /* Integration de la gestion du temps */

int *buffer; //On crée une variable buffer, dont on va définir la taille, et qui va acceuillir le texte envoyé par le client. 

void traitement_message(int len, int client_pid)
{
	char message_propre[len + 1]; //On crée la variable message_propre, qui accueille nos caractères pour ensuite être affichée. 

	int iterator = 0;
	while (iterator < len){
		message_propre[iterator] = *(buffer + iterator);
		iterator++;
	}

	message_propre[len] = '\0'; //On ajoute le symbole de fin après le message. 

    time_t current_time;
    time(&current_time); //On stocke le temps actuel dans une variable current_time

    char temps[20];
    strftime(temps, sizeof(temps), "%Y-%m-%d %H:%M:%S", localtime(&current_time)); //On formate le temps sous la forme souhaitée. 

    printf("%s : ", temps); //Affichage du temps
	printf("%s \n", message_propre); //Affichage du message

	FILE *fichierConversation = fopen("conversations.log", "a"); // Ouverture du fichier en mode "ajout"
	if (fichierConversation == NULL) {
    	perror("Nous n'avons pas pu ouvrir ni créer le fichier de logs");
    	exit(1);
	}
	fprintf(fichierConversation, "FROM CLIENT[%d", client_pid);
	fprintf(fichierConversation, "] ");
	fprintf(fichierConversation, "%s - %s\n", temps, message_propre); // Écriture dans le fichier avec le format spécifié
	fclose(fichierConversation); // Fermeture du fichier
}

void sig_handler(int sig, siginfo_t* info, void* vp) //fonction qui va s'activer à la reception du signal client.
{
	int lettre = info->si_value.sival_int; // Cette ligne permet de récupérer chaque caractères incluts dans le SIGUSR1 en tant que value.

	*buffer = lettre; // chaque lettre est ensuite placée dans l'ordre dans la variable buffer


	if (*buffer == '\0') //On repère la fin du message envoyé par le client
	{
		int len = *(buffer - 1);
		int client_pid = *(buffer - len - 2);

		buffer -= len + 1; // On se place sur la nouvelle première case du tableau buffer, afin de commencer à traiter le prochain message, qui écrasera ensuite l'ancien.
		printf("FROM CLIENT[%d", client_pid); //affiche le PID du client
		printf("] "); 
		traitement_message(len, client_pid); //On traite le message terminé.
		
	}
	else
		buffer++; // le buffer se place à la case suivante
}

void allocation_memoire() //Fonction servant à associer de la mémoire à la variable globale buffer. 
{

	buffer = malloc(1300 * sizeof *buffer);
}

int main(int argc, char const *argv[])
{
	printf("Miniteams starting...\n");
	printf("My PID is %i\n", getpid());
	printf("Waiting for new messages\n");
	allocation_memoire();
	struct sigaction gestion_signal; // On crée la variable gestion_signal, qui va préciser les actions à effectuer à la reception du signal.
	gestion_signal.sa_flags = SA_SIGINFO; //On renseigne le fait que le handler doit prendre en compte l'info du signal en plus du signal seul. 
	gestion_signal.sa_sigaction = sig_handler; //On renseigne la fonction sig_handler comme la fonction a executer lors de la reception du signal USR1.

	sigaction(SIGUSR1, &gestion_signal, NULL); //Cette ligne indique que le signal SIGUSR1 est receptionné et traité par la variable gestion signal.

	while(1)
	{
		pause(); //Creation d'une boucle
	}

	free(buffer); //On relache le buffer. 
	return 0;
}
