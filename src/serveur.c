#include "serveur.h"

int journal;
int nbFichiers;
sFileList *fichiers;
sem_t semWorkers;
int continuer;

int main(int argc, char *argv[])
{
	//Déclarations
	DataSpec workers[NBWORKERS];
	int modeLog, i, retour, ecoute, canal;
	struct sockaddr_in adrEcoute, reception;
	socklen_t receptionLen; 
	char buf[LIGNE_MAX];

	fichiers = initFileList();
	//receptionLen = malloc(sizeof(socklen_t));
	memset(&reception, 0, sizeof(reception));
	canal = i = retour = nbFichiers = ecoute = 0;
	continuer = 1;
	modeLog = O_WRONLY|O_APPEND|O_CREAT;
	sem_init(&semWorkers, 0, NBWORKERS);
	
	//Ouverture du journal de log
	journal = open("journal.log",modeLog, 0660);
	if (journal == -1)
	{
		erreur_IO("open journal");
	}
	
	//Socket
	ecoute = socket (AF_INET, SOCK_STREAM, 0);
	if (ecoute < 0) {
		erreur_IO("socket");
	}
	
	adrEcoute.sin_family = AF_INET;
	adrEcoute.sin_addr.s_addr = INADDR_ANY;
	adrEcoute.sin_port = htons(PORT);
	retour = bind (ecoute,  (struct sockaddr *) &adrEcoute, sizeof(adrEcoute));
	if (retour < 0)
	{
		erreur_IO("bind");
	}

	retour = fcntl (ecoute, F_GETFL, 0);
	fcntl(ecoute, F_SETFL, retour | O_NONBLOCK);
	
	retour = listen (ecoute, 20);
	if (retour < 0) 
	{
		erreur_IO("listen");
	}
	
	//Initialisation du pool de workers
	for (i=0; i<NBWORKERS ; i++)
	{
		workers[i].tid = i;
		workers[i].libre = VRAI;
		workers[i].quit = FAUX;
		workers[i].canal = -1;
		sem_init(&workers[i].sem,0,0);
		retour = pthread_create(&workers[i].id, NULL, traitement, &workers[i]);
		if (retour != 0)
		{
			erreur_IO("pthread_create");
		}else{
			printf("Thread lancé !\n");
		}
	}
	
	//Traitement des connexions
	while (continuer)
	{
		canal = accept(ecoute, (struct sockaddr *) &reception, &receptionLen);
		if (canal < 0 && errno != EAGAIN) 
		{
			erreur_IO("accept");
		}
		usleep(500);
		if(errno != EAGAIN){
			printf("Nouveau client !\n");
			sem_wait(&semWorkers);
			for(i = 0; i < NBWORKERS; i++)
				if(workers[i].libre)
					break;
			
			workers[i].canal = canal;
			workers[i].clientIP = ntohl(reception.sin_addr.s_addr);
			snprintf(buf, LIGNE_MAX, "Nouveau \t(%u)\t : %s\n", reception.sin_addr.s_addr, stringIP(ntohl(reception.sin_addr.s_addr)));
			printd(buf);
			sem_post(&workers[i].sem);
		}
		errno = ENOENT;
  	}
  	
  	for(i = 0; i < NBWORKERS; i++){
  		workers[i].quit = VRAI;
		sem_post(&workers[i].sem);
  	}
  	
  	for(i = 0; i < NBWORKERS; i++){
  		pthread_join(workers[i].id, NULL);
  	}

	free(fichiers);
	return 0;
}

void *traitement(void *arg)
{
	//Déclarations
	DataSpec *data = (DataSpec *) arg;
	int requete;
	
	//attente d'un canal
	while (VRAI)
	{
		sem_wait(&data->sem);

		if(data->quit){
			if(data->canal >= 0)
				finConnexion(data);
			break;
		}
		
		//Quand le canal est attribué
		data->libre = FAUX;
		
		//Côté client, affichage d'un menu :
		//1. recherche 2. affichage des fichiers disponibles 0. fin de la connexion
		//(Options non affichées : 3. nouvelle connexion/annonce possession d'un/plusieurs nouveau(x) fichier(s))
		requete = -1;
		//Réception du choix fait à partir du menu
		read(data->canal, &requete, sizeof(int));
		
		switch(requete)
		{
			case 1:
				traitementRecherche(data);
				break;
			case 2:
				affichageFichiers(data);
				break;
			case 3:
				traitementAnnonce(data);
				break;
			case 4:
				data->quit = VRAI;
				continuer = FAUX;
			case 0:
				finConnexion(data);
				break;
			default:
				break;				
		}
		data->libre = VRAI;
		sem_post(&semWorkers);
	}
	printf("C'est la fin pour moi !\n");
	pthread_exit(NULL);
}

void traitementRecherche(DataSpec *data)
{
	//Déclarations
	// char recherche[LIGNE_MAX]="";
	// int nbTrouves = 0;
	// listeFichiers *resultat, *copie;
	// fichierSimple *envoi;
	// int i=0;
	// int numFichier = 0;
	// clientsDL *clients;
	// clients = (clientsDL*) malloc(sizeof(clientsDL));
	// clients->pairs = malloc(NBDOWNLOAD*sizeof(adresseIP));
	// adresseIP *pairs;
	
	// //Réception du choix fait à partir du menu
	// read(data->socketID, &recherche, LIGNE_MAX*sizeof(char));
	// resultat = rechercheFichier(fichiers,recherche,&nbTrouves);
	// copie = resultat;
	
	// //Envoi du nombre de fichiers trouvés
	// write(data->socketID, &nbTrouves, sizeof(int));
	// if (nbTrouves!=0)
	// {
	// 	//Conversion de la liste en un tableau
	// 	envoi = malloc(nbTrouves*sizeof(fichierSimple));
	// 	for (i=0; i<nbTrouves ; i++)
	// 	{
	// 		if (resultat == NULL)
	// 			erreur_IO("fonction recherche");
	// 		envoi[i].id = resultat->id;
	// 		strcpy(envoi[i].nom,resultat->nom);
	// 		resultat = resultat->suiv;
	// 	}
		
	// 	//Envoi le tableau de résultat
	// 	write(data->socketID,resultat, nbTrouves*sizeof(fichierSimple));
		
	// 	//Récupère le numéro du fichier voulu par le client
	// 	read(data->socketID, &numFichier, sizeof(int));
		
	// 	//Traitement de la demande
	// 	if (numFichier != 0) //Si le client veut un fichier
	// 	{
	// 		i = 0;
	// 		while (i < numFichier)
	// 		{
	// 			if (copie == NULL)
	// 				erreur_IO("recherche");
					
	// 			copie = copie->suiv;
	// 			i++;
	// 		}
	// 		clients = envoiPairs(copie, NULL);
	// 		pairs = malloc(clients->nbPairs*sizeof(adresseIP));
	// 		for (i = 0; i < clients->nbPairs ; i++)
	// 			strcpy(pairs[i].IP,clients->pairs[i].IP);
				
	// 		//Annonce du nombre de pairs
	// 		write(data->socketID, &clients->nbPairs, sizeof(int));
	// 		//Envoi des pairs
	// 		write(data->socketID, pairs, clients->nbPairs*sizeof(adresseIP));
	// 	}
	// }
	printf("Recherche de fichiers !\n");
}

void traitementAnnonce(DataSpec *data)
{
	//Déclarations
	size_t filesSize;
	int recvSize;
	sFileTab fileTab;

	recv(data->canal, &recvSize, sizeof(recvSize), 0);
	filesSize = recvSize*sizeof(sFile);
	fileTab.length = recvSize;
	fileTab.tab = malloc(filesSize);
	recv(data->canal, fileTab.tab, filesSize, 0);

	announceFiles(fichiers, fileTab, data->clientIP);
	free(fileTab.tab);
	printFileList(fichiers);
	printf("Annonce des fichiers d'un client !\n");
}

void affichageFichiers(DataSpec *data)
{
	//Déclarations
	// fichierSimple *envoi;
	// int i = 0;
	// listeFichiers *resultat;
	// resultat = fichiers;
	
	// envoi = malloc(nbFichiers*sizeof(fichierSimple));
	
	// //Conversion de la liste en un tableau
	// for (i=0; i<nbFichiers ; i++)
	// {
	// 	if (resultat == NULL)
	// 		erreur_IO("fonction recherche");
	// 	envoi[i].id = resultat->id;
	// 	strcpy(envoi[i].nom,resultat->nom);
	// 	resultat = resultat->suiv;
	// }
	
	// //Annonce nombre de fichiers
	// write(data->socketID,&nbFichiers,sizeof(int));
	// //envoi tab
	// write(data->socketID, envoi, nbFichiers*sizeof(fichierSimple));
	printf("Affichage des fichiers présents.\n");
}

void finConnexion(DataSpec *data)
{
	//suppressionIPFichiers(fichiers, data->IPClient);
	close(data->canal);
	data->canal = -1;
}
