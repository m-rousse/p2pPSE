#include "serveur.h"

int journal;
int nbFichiers;
sFileList *fichiers;
sem_t semWorkers;
int continuer;
pthread_mutex_t mtxFileList = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
	//Déclarations
	DataSpec workers[NBWORKERS];
	int modeLog, i, retour, ecoute, canal;
	struct sockaddr_in adrEcoute, reception;
	socklen_t receptionLen; 

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
				disconnectClient(data);
			break;
		}
		
		//Quand le canal est attribué
		data->libre = FAUX;
		
		requete = -1;
		//Réception du choix fait à partir du menu
		read(data->canal, &requete, sizeof(int));
		
		//printf("Commande : %d\n", requete);
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
			case 7:
				data->quit = VRAI;
				continuer = FAUX;
				quitServer(data);
				break;
			case 8:
				disconnectClient(data);
				break;
			case 9:
				sendFileClient(data);
				break;
			case 10:
				printDebug(data);
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
	char *search;
	int numRes, numRead;
	sFileTab *result;

	numRead = -1;
	search = malloc(LIGNE_MAX*sizeof(char));
	memset(search, 0, LIGNE_MAX*sizeof(char));
	//Réception du choix fait à partir du menu
	numRead = recv(data->canal, search, LIGNE_MAX*sizeof(char), 0);
	if(numRead < 0)
		printd("Il y a une erreur.\n");
	printf("Terme de la recherche : %s\n", search);
	result = searchFileList(fichiers,search);
	numRes = result->length;

	//Envoi du nombre de fichiers trouvés
	write(data->canal, &numRes, sizeof(int));
	if(numRes > 0)
		write(data->canal, result->tab, numRes*sizeof(sFile));
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

	pthread_mutex_lock(&mtxFileList);
	announceFiles(fichiers, fileTab, data->clientIP);
	printFileList(fichiers);
	pthread_mutex_unlock(&mtxFileList);
	free(fileTab.tab);
	printf("Annonce des fichiers d'un client !\n");
}

void affichageFichiers(DataSpec *data)
{
	// Déclarations
	sFile *walk;
	size_t ftSize;
	sFileTab *ft;

	walk = fichiers->first;
	ftSize = 0;
	ft = malloc(sizeof(sFileTab));
	ft->length = 0;
	ft->tab = NULL;

	while(walk != NULL){
		ftSize += sizeof(sFile);
		ft->tab = realloc(ft->tab, ftSize);
		strcpy(ft->tab[ft->length].name,walk->name);
		ft->tab[ft->length].id = walk->id;
		ft->length++;
		walk = walk->next;
	}

	//Annonce nombre de fichiers
	write(data->canal,&ft->length,sizeof(int));
	//envoi tab
	write(data->canal, ft->tab, ftSize);
	printf("Affichage des fichiers présents.\n");
}

void sendFileClient(DataSpec *data)
{
	// //Déclarations
	sClient 	*cWalk;
	sFile 		*fWalk;
	size_t 		ctSize;
	sClientTab 	*ct;
	int 		fileID, numRead;

	fWalk = fichiers->first;
	ctSize = 0;
	ct = malloc(sizeof(sClientTab));
	ct->length = 0;
	ct->tab = NULL;

	numRead = recv(data->canal, &fileID, sizeof(int),0);
	if(numRead < 0)
		; // erreur
	while(fWalk != NULL){
		if(fWalk->id == fileID){
			cWalk = fWalk->clients.first;
			while(cWalk != NULL){
				ctSize += sizeof(sClient);
				ct->tab = realloc(ct->tab, ctSize);
				ct->tab[ct->length].IP = cWalk->IP;
				ct->length++;
				cWalk = cWalk->next;
			}
			break;
		}
		fWalk = fWalk->next;
	}

	//Annonce nombre de clients
	write(data->canal,&ct->length,sizeof(int));
	//envoi tab
	if(ct->length > 0)
		write(data->canal, ct->tab, ctSize);
	printf("Envoi des clients.\n");
}

void quitServer(DataSpec *data)
{
	freeFileList(fichiers);
	close(data->canal);
	data->canal = -1;
}

void disconnectClient(DataSpec *data)
{
	deleteIPFileList(fichiers, data->clientIP);
	close(data->canal);
	data->canal = -1;
}

void printDebug(DataSpec *data){
	char buf[LIGNE_MAX];
	read(data->canal, buf, LIGNE_MAX*sizeof(char));
	printd(buf);
}