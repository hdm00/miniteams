Projet d'école, création d'un moyen de communication entre deux machines sur un même réseau. 

V1 du projet - version miniteams : Fonctionnement avec des signaux, messages placés en tant que valeur des signaux personnalisés SIGUSR.

V2 du projet - version myteams : Fonctionnement inter-réseau avec les sockets.

server.c et client.c a compiler avec gcc server.c -o server / gcc client.c -o client
Puis a executer avec ./server et ./client

Serveur définit en 127.0.0.1 4242 par défaut, l'ip, port, pseudo et message sont à passer en arguement du client tel que : 

./client 1270.0.0.1 4242 "Hugo" "Bonjour à tous".
