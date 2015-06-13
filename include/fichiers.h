#ifndef FICHIERS_H
#define FICHIERS_H

#include "pse.h"


#define NBWORKERS 5 //Nombre de workers du serveur
#define ATTENTE 2000 //Délais d'attente
#define NBDOWNLOAD 5 //Nombre de clients auprès desquels le client va télécharger un fichier

typedef struct listeClients{
	char adrIP[16]; //adresse IP du client
	struct listeClients *suiv; //client suivant dans la liste
} listeClients;

typedef struct listeFichiers{
	int id; //Identifiant du fichier
	char *nom; //Nom du fichier
	int nbClients; //Nombre de clients possédant le fichier
	listeClients *clients; //Liste des clients possédant le fichier
	struct listeFichiers *suiv;
} listeFichiers;

//Structure simple permettant l'envoi au client
typedef struct fichierSimple{
	int id; //Identifiant du fichier
	char nom[LIGNE_MAX]; //Nom du fichier
} fichierSimple;

//structure envoyée au client (avec le nombre d'éléments dans le tableau)
typedef struct tabFichiers{
	fichierSimple *fichiers; //tableau contenant les fichiers
	int nbFichiers; //nombre de fichiers
} tabFichiers;

//Type adresse IP
typedef struct adresseIP {
	char IP[16];
} adresseIP;

//Ce que le serveur renvoi au client après une recherche
typedef struct clientsDL{
	adresseIP *pairs; //tableau contenant les adresses IP
	int nbPairs; //Nombre d'adresses IP envoyées
} clientsDL;

extern int nbFichiers;

void initClients(listeClients *clients);
void initFichiers(listeFichiers *fichiers);
void initClientsDL(clientsDL *liste);
listeFichiers *rechercheFichier(listeFichiers *fichiers, char *recherche, int *resultats);
listeClients *suppressionClientNum(listeClients *clients, int num);
listeClients *clientNum(listeClients *clients, int num);
int annonceFichier(listeFichiers *fichiers, tabFichiers fichiersClient, adresseIP IPClient);
listeClients *ajoutClient(listeClients *clients, adresseIP IPClient);
listeFichiers *ajoutFichier(listeFichiers *fichiers, fichierSimple f, adresseIP IPClient);
int freeFichiers(listeFichiers *fichiers);
int suppressionIPFichiers(listeFichiers *fichiers, char *adrIP);
int suppressionClientIP(listeClients *clients, char *adrIP);
clientsDL *envoiPairs(listeFichiers *fichier, clientsDL *pairsConnus);
int suppressionFichierListe(listeFichiers *p, listeFichiers *fichiers);

#endif