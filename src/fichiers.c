#include "fichiers.h"

//Initialisation de liste des clients
void initClients(listeClients *clients)
{
	clients = malloc(sizeof(listeClients));
	strcpy(clients->adrIP, "");
	clients->suiv = NULL;
}

//Initialisation de la liste des fichiers
void initFichiers(listeFichiers *fichiers)
{
	fichiers->id = -1;
	fichiers->nom = malloc(LIGNE_MAX*sizeof(char));
	strcpy(fichiers->nom,"");
	fichiers->nbClients = 0;
	initClients(fichiers->clients);
	fichiers->suiv = NULL;
}

//Recherche d'un fichier
//Prend en argument la liste des fichiers, le nom du fichier recherché
//Si des fichiers correspondent, renvoie une liste de fichiers et resultats contient le nombre de fichiers correspondant
//Si le fichier n'est pas trouvé, renvoie NULL
listeFichiers *rechercheFichier(listeFichiers *fichiers, char *recherche, int *resultats)
{
	//Déclarations
	listeFichiers *f = fichiers; //Permet le parcours de la liste de fichiers
	listeFichiers *result = NULL; //Structure à renvoyer
	
	while (f!=NULL)
	{
		if (strstr(f->nom,recherche)!=NULL)
		{
			if (result == NULL)
			{
				result = f;
				result->suiv = NULL;
			}
			else
			{
				result->suiv = f;
				result = result->suiv;
				result->suiv = NULL;
			}
		}
		f = f->suiv;
	}
	return result;
}

//pour fichier, renvoie une liste de pairs
//si le client connait déjà des pairs, ils sont à placer dans pairsConnus
//(cela évite de renvoyer les mêmes pairs)
//si le client n'a aucun pair, pairsConnus = NULL
clientsDL *envoiPairs(listeFichiers *fichier, clientsDL *pairsConnus)
{
	//Déclarations
	int i = 0;
	listeClients *c; //Une fois le fichier trouvé, permet le parcours de la liste des clients le possédant
	listeClients *retour; //Client à ajouter dans liste
	adresseIP *tabPairs; //tableau contenant les adresses IP
	clientsDL *result; //Structure à renvoyer
	int nb = 0; //Servira de compteur pour le nombre de clients possibles restant
	int numClient = 0;
	

	c = fichier->clients;
	i = 0;
	result = (clientsDL*) malloc(sizeof(clientsDL));

	//si le client ne connait aucun pair
	if (pairsConnus == NULL)
	{
		nb = fichier->nbClients;
	}
	else
	{
		//si le client connait certains pairs
		//on les supprime de la liste
		for (i = 0; i < pairsConnus->nbPairs; i++)
			suppressionClientIP(c, pairsConnus->pairs[i].IP);
			
		nb = fichier->nbClients - pairsConnus->nbPairs;
		
		//si le client connait déjà tous les pairs
		if (fichier->nbClients <= pairsConnus->nbPairs)
			return NULL;
	}
		
	if (NBDOWNLOAD >= nb) //Si le nombre de clients possédant le fichier (et non connus du client) est inférieur à NBDOWNLOAD
	//On envoie toutes les adresses restantes IP au client
	{
		tabPairs = malloc(nb*sizeof(adresseIP));
		for (i=0 ; i < nb ; i++)
		{
			strcpy(tabPairs[i].IP,c->adrIP);
			c = c->suiv;
		}
		result->nbPairs = fichier->nbClients;
		result->pairs = tabPairs;
		return result;
	}
	else
	//Sinon, on choisit de façon aléatoire NBDOWNLOAD adresses IP
	{
		tabPairs = malloc(NBDOWNLOAD*sizeof(adresseIP));
		for (i=0 ; i < NBDOWNLOAD ; i++)
		{
			numClient = rand()%(nb) + nb;
			retour = clientNum(c,numClient); //Recherche d'un client aléatoire
			strcpy(tabPairs[i].IP,retour->adrIP); //Copie de son adresse IP
			suppressionClientNum(c,numClient); //Supprime le client de la liste
			nb--; //Décrémente le nombre de clients encore possibles
		}
		result->nbPairs = NBDOWNLOAD;
		result->pairs = tabPairs;
		return result;
	}
}

//Retourne un pointeur vers le client placé en position num de clients
//(Si la liste est trop courte, retourne NULL)
listeClients *clientNum(listeClients *clients, int num)
{
	//Déclarations
	int compteur = 0;
	listeClients *copie = clients;
	
	while (copie!=NULL && compteur < num)
	{
		copie = copie->suiv;
		compteur++;
	}
		
	if (compteur == num)
		return copie;
	else
		return NULL;
}

//Supprime le client en position num de la liste
//(Si la liste est trop courte, ne supprime rien)
listeClients *suppressionClientNum(listeClients *clients, int num)
{
	//Déclarations
	int compteur = 0;
	listeClients *precedent = NULL;
	listeClients *actu = clients;
	
	if (num==1)
		return clients->suiv;
	
	while (actu!=NULL && compteur < num)
	{
		precedent = actu;
		actu = actu->suiv;
		compteur++;
	}
	
	if (compteur == num && actu!=NULL)
	{
		precedent->suiv = actu->suiv;
		free(actu);
	}
	return clients;
}



//Supprime adrIP des clients connectés en mettant à jour la liste de fichiers
//Retourne 0 s'il est trouvé, 1 sinon
int suppressionIPFichiers(listeFichiers *fichiers, char *adrIP)
{
	//Déclarations
	listeFichiers *p = fichiers; //parcours de la liste de fichiers
	int retour;
	
	while (p!=NULL)
	{
		retour = suppressionClientIP(p->clients,adrIP);
		if (!retour) //s'il a été trouvé
		{
			(p->nbClients)--; //on décrémente le nombre de clients possédant le fichier
			if (p->nbClients == 0) //si aucun client ne possède le fichier
			{
				suppressionFichierListe(p,fichiers); //on le supprime de la liste de fichiers
				nbFichiers--;
			}
		}
		p = fichiers->suiv;
	}
	return retour;
}

//supprime le fichier p de fichiers
//retourne 0 si l'opération a fonctionné, 1 sinon
int suppressionFichierListe(listeFichiers *p, listeFichiers *fichiers)
{
	//Déclarations
	listeFichiers *actu = fichiers;
	listeFichiers *precedent = NULL;
	
	if (p==fichiers) //si c'est le premier de la liste
	{
		fichiers = fichiers->suiv;
		return 0;
	}
	
	while (actu!=NULL)
	{
		if (actu==p)
		{
			precedent->suiv = actu->suiv;
			free(actu);
			return 0;
		}
		precedent = actu;
		actu = actu->suiv;
	}
	
	return 1;
		
}

//Supprime (s'il est dans la liste de clients) le client dont l'adresse IP est adrIP
//Retourne 0 s'il est trouvé, 1 sinon
int suppressionClientIP(listeClients *clients, char *adrIP)
{
	//Déclaration
	listeClients *precedent = NULL;
	listeClients *actu = clients;
	
	if(actu == NULL)
		return 1;

	if (!strcmp(actu->adrIP,adrIP)) //Si c'est le premier client de la liste
	{
		clients = clients->suiv;
		return 0;
	}
	
	while (actu!=NULL)
	{
		if (!strcmp(actu->adrIP,adrIP))
		{
			precedent->suiv = actu->suiv;
			free(actu);
			return 0;
		}
		precedent = actu;
		actu = actu->suiv;
	}
	
	return 1;
}


//Ajout des fichiers possédés par le client dans la liste de fichiers
int annonceFichier(listeFichiers *fichiers, tabFichiers fichiersClient, adresseIP IPClient)
{
	//Déclarations
	int nbFichiersAjout = fichiersClient.nbFichiers;
	int *traite;
	traite = malloc(nbFichiersAjout * sizeof(int)); //Nous permet de savoir si le fichier a déjà été ajouté
	listeFichiers *f = fichiers; //Parcours de fichiers
	int i = 0;
	
	//Initialisation
	for (i = 0 ; i < nbFichiersAjout ; i++)
		traite[i] = 0;
	
	while (f!=NULL)
	{
		//Parcours du tableau des fichiers à ajouter
		i = 0;
		while (i<nbFichiersAjout)
		{
			//Si le fichier est déjà dans la liste, on ajoute uniquement l'adresse IP du client
			if (!strcmp(fichiersClient.fichiers[i].nom, f->nom))
			{
				f->clients = ajoutClient(f->clients, IPClient);
				f->nbClients++;
				traite[i] = 1;
				i = nbFichiersAjout; //Permet d'arrêter le parcours des fichiers (1 fichier n'est présent qu'une seule fois)
			}
			i++;
		}
		
		f = f->suiv;
	}
	
	
	f = fichiers;
	for (i=0 ; i<nbFichiersAjout ; i++)
	{
		//Si le fichier n'était pas dans la liste, on l'ajoute au début
		if (!traite[i])
		{
			ajoutFichier(f,fichiersClient.fichiers[i], IPClient);
			nbFichiers++;
		}
	}
	
	return 0;
}

//Ajout d'un client dans la liste de clients
listeClients *ajoutClient(listeClients *clients, adresseIP IPClient)
{
	//Déclarations
	listeClients *nvClient;
	nvClient = malloc(sizeof(listeClients));
	
	strcpy(nvClient->adrIP, IPClient.IP);
	nvClient->suiv = clients;
	
	return nvClient;
}

//Ajout d'un fichier au début de la liste
listeFichiers *ajoutFichier(listeFichiers *fichiers, fichierSimple f, adresseIP IPClient)
{
	//Déclarations
	listeFichiers *nvFichier;
	nvFichier = malloc(sizeof(listeFichiers));
	
	nvFichier->id = f.id;
	strcpy(nvFichier->nom, f.nom);
	nvFichier->nbClients = 1;
	nvFichier->clients = ajoutClient(NULL,IPClient);
	nvFichier->suiv = fichiers;
	
	return nvFichier;
}

//libère l'espace mémoire de la liste de fichiers
//(Pour la déconnexion)
int freeFichiers(listeFichiers *fichiers)
{
	//Déclarations
	listeFichiers *f1, *f2;
	listeClients *c1, *c2;
	
	
	//Arrêt si la liste est vide
	if (fichiers == NULL)
		return 0;
	
	f1 = fichiers;
	f2 = fichiers->suiv;
	
	while (f1 != NULL)
	{
		//Libération de la liste de clients
		c1 = fichiers -> clients;
		c2 = c1->suiv;
		while (c1 != NULL)
		{
			free(c1);
			c1 = c2;
			if (c1!=NULL)
				c2 = c1->suiv;
		}
		
		free(f1);
		f1 = f2;
		if (f1!=NULL)
			f2 = f1->suiv;
	}
	return 0;
}
