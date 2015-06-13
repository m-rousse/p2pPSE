#include "serveur.h"

int journal;
int nbFichiers;
listeFichiers *fichiers;

int main(int argc, char *argv[])
{
	//Déclarations
	DataSpec workers[NBWORKERS];
	int modeLog = O_WRONLY|O_APPEND|O_CREAT;
	int i = 0;
	int retour = 0;
	int continuer = 1;
	fichiers = malloc(sizeof(listeFichiers));
	nbFichiers = 0;
	
	//Erreur sur les arguments
	if (argc != 2)
	{
		erreur("usage %s port\n", argv[0]);
	}
	
	//Initialisation
	initFichiers(fichiers);
	
	//Ouverture du journal de log
	journal = open("journal.log",modeLog, 0660);
	if (journal == -1)
	{
		erreur_IO("open journal");
	}
	
	//Initialisation du pool de workers
	for (i=0; i<NBWORKERS ; i++)
	{
		workers[i].tid = i;
		workers[i].libre = 1;
		workers[i].canal = -1;
		retour = pthread_create(&workers[i].id, NULL, traitement, &workers[i]);
		if (retour != 0)
		{
			erreur_IO("pthread_create");
		}
	}
	
	free(fichiers);
	return 0;
}

void *traitement(void *arg)
{
	//Déclarations
	DataSpec *data = (DataSpec *) arg;
	struct sockaddr_in adrEcoute, reception;
	int mode = O_WRONLY | O_APPEND | O_CREAT | O_TRUNC;
	int fin = 0;
	int requete;
	int retour = 0;
	int ecoute = 0;
	
	//attente d'un canal
	while (1)
	{
		while (data->canal == -1)
		{
			//attente
			usleep(ATTENTE);
		}
		
		//Quand le canal est attribué
		data->libre = 0;
		fin = 0;
		
		//Socket
		ecoute = socket (AF_INET, SOCK_STREAM, 0);
		if (ecoute < 0) {
			erreur_IO("socket");
		}
		data->socketID = ecoute;
		
		adrEcoute.sin_family = AF_INET;
		adrEcoute.sin_addr.s_addr = INADDR_ANY;
		adrEcoute.sin_port = htons(PORT);
		retour = bind (ecoute,  (struct sockaddr *) &adrEcoute, sizeof(adrEcoute));
		if (retour < 0)
		{
			erreur_IO("bind");
		}
		
		retour = listen (ecoute, 20);
		if (retour < 0) {
			erreur_IO("listen");
		}

	//Côté client, affichage d'un menu :
	//1. recherche 2. affichage des fichiers disponibles 0. fin de la connexion
	//(Options non affichées : 3. nouvelle connexion/annonce possession d'un/plusieurs nouveau(x) fichier(s))

		
		while (!fin)
		{
			//Réception du choix fait à partir du menu
			read(ecoute, &requete, sizeof(int));
			
			switch(requete)
			{
				case 1:
					retour = pthread_create(&data->id, NULL, traitementRecherche, &data);
					if (retour != 0)
					{
						erreur_IO("pthread_create");
					}
				break;
				case 2:
					retour = pthread_create(&data->id, NULL, affichageFichiers, &data);
					if (retour != 0)
					{
						erreur_IO("pthread_create");
					}
				break;
				case 3:
					retour = pthread_create(&data->id, NULL, traitementAnnonce, &data);
					if (retour != 0)
					{
						erreur_IO("pthread_create");
					}
				break;
				case 0:
					retour = pthread_create(&data->id, NULL, finConnexion, &data);
					if (retour != 0)
					{
						erreur_IO("pthread_create");
					}
					fin = 0;
				break;
				default:
				break;				
			}
		}
	}
	pthread_exit(NULL);
}

void *traitementRecherche(void *arg)
{
	//Déclarations
	char recherche[LIGNE_MAX]="";
	DataSpec *data = (DataSpec *) arg;
	int nbTrouves = 0;
	listeFichiers *resultat, *copie;
	fichierSimple *envoi;
	int i=0;
	int numFichier = 0;
	clientsDL *clients;
	clients->pairs = malloc(NBDOWNLOAD*sizeof(adresseIP));
	adresseIP *pairs;
	
	//Réception du choix fait à partir du menu
	read(data->socketID, &recherche, LIGNE_MAX*sizeof(char));
	resultat = rechercheFichier(fichiers,recherche,&nbTrouves);
	copie = resultat;
	
	//Envoi du nombre de fichiers trouvés
	write(data->socketID, &nbTrouves, sizeof(int));
	if (nbTrouves!=0)
	{
		//Convertion de la liste en un tableau
		envoi = malloc(nbTrouves*sizeof(fichierSimple));
		for (i=0; i<nbTrouves ; i++)
		{
			if (resultat == NULL)
				erreur_IO("fonction recherche");
			envoi[i].id = resultat->id;
			strcmp(envoi[i].nom,resultat->nom);
			resultat = resultat->suiv;
		}
		
		//Envoi le tableau de résultat
		write(data->socketID,resultat, nbTrouves*sizeof(fichierSimple));
		
		//Récupère le numéro du fichier voulu par le client
		read(data->socketID, &numFichier, sizeof(int));
		
		//Traitement de la demande
		if (numFichier != 0) //Si le client veut un fichier
		{
			i = 0;
			while (i < numFichier)
			{
				if (copie == NULL)
					erreur_IO("recherche");
					
				copie = copie->suiv;
				i++;
			}
			clients = envoiPairs(copie, NULL);
			pairs = malloc(clients->nbPairs*sizeof(adresseIP));
			for (i = 0; i < clients->nbPairs ; i++)
				strcpy(pairs[i].IP,clients->pairs[i].IP);
				
			//Annonce du nombre de pairs
			write(data->socketID, &clients->nbPairs, sizeof(int));
			//Envoi des pairs
			write(data->socketID, pairs, clients->nbPairs*sizeof(adresseIP));
		}
	}
	pthread_exit(NULL);
}

void *traitementAnnonce(void *arg)
{
	//Déclarations
	DataSpec *data = (DataSpec *) arg;
	int nbAnnonce = 0;
	fichierSimple *inter;
	tabFichiers fichiersClient;
	
	//Annonce nombre de fichiers
	read(data->socketID, &nbAnnonce, sizeof(int));
	//Réception du tableau
	inter = malloc(nbAnnonce*sizeof(fichierSimple));
	read(data->socketID, &inter, sizeof(nbAnnonce*sizeof(fichierSimple)));
	fichiersClient.fichiers = inter;
	fichiersClient.nbFichiers = nbAnnonce;
	//Récupérer adresse IP du client ????
	annonceFichier(fichiers, fichiersClient, IPClient);
	pthread_exit(NULL);
}

void *affichageFichiers(void *arg)
{
	//Déclarations
	DataSpec *data = (DataSpec *) arg;
	fichierSimple *envoi;
	int i = 0;
	listeFichiers *resultat;
	resultat = fichiers;
	
	envoi = malloc(nbFichiers*sizeof(fichierSimple));
	
	//Conversion de la liste en un tableau
	for (i=0; i<nbFichiers ; i++)
	{
		if (resultat == NULL)
			erreur_IO("fonction recherche");
		envoi[i].id = resultat->id;
		strcmp(envoi[i].nom,resultat->nom);
		resultat = resultat->suiv;
	}
	
	//Annonce nombre de fichiers
	write(data->socketID,&nbFichiers,sizeof(int));
	//envoi tab
	write(data->socketID, envoi, nbFichiers*sizeof(fichierSimple));
}

void *finConnexion(void *arg)
{
	//Déclarations
	DataSpec *data = (DataSpec *) arg;
}
