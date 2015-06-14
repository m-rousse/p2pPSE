#include <filelist.h>

extern pthread_mutex_t mtxFileList;

sFileList* initFileList(){
	sFileList* fl = (sFileList*) malloc(sizeof(sFileList));
	fl->first = NULL;
	return fl;
}

void initFile(sFile *f){
	f->id = -1;
	memset(f->name, 0, LIGNE_MAX*sizeof(char));
	f->next = NULL;
	f->exists = FAUX;
	initClientList(&f->clients);
}

void initClientList(sClientList *cl){
	cl->length = 0;
	cl->first = 0;
}

void initClient(sClient *c){
	c->IP = 0;
	c->next = NULL;
}

void restoreFileList(sFileList* fl){
	char 	buf[LIGNE_MAX];
	int 	id, len;
	int 	fd; // File descriptor
	sFile 	*tmp;

	printd("restoreFileList : Debut");
	fd = open(SAVEFILE,O_RDONLY);
	if(fd >= 0){
		while((len = lireLigne(fd, buf)) > 0){
			id = atoi(buf);
			len = lireLigne(fd, buf);
			if(len > 0){
				tmp = createFile(buf, id);
				tmp->exists = 1;
				addFileToFileList(fl,tmp);
			}else{
				printd("restoreFileList : Fichier malformé");
			}
		}
	}
	printd("restoreFileList : Fin");
}

void saveFileList(sFileList* fl){
	int fd; // File descriptor
	sFile* walk;
	char idString[LIGNE_MAX] = "";

	printd("saveFileList : Debut");
	fd = open(SAVEFILE, O_WRONLY | O_TRUNC | O_CREAT);
	if(fd >= 0){
		if(fl->first != NULL){
			walk = fl->first;
			do{
				if(walk->exists){
					snprintf(idString, LIGNE_MAX, "%d", walk->id);
					ecrireLigne(fd, idString);
					ecrireLigne(fd, walk->name);
				}
				walk = walk->next;
			}while(walk != NULL);
		}
	}
	printd("saveFileList : Fin");
}

sFile* createFile(char* name, int id){
	sFile* f;

	f = (sFile*) malloc(sizeof(sFile));
	initFile(f);
	strncpy(f->name, name, LIGNE_MAX);
	f->id = id;
	return f;
}

void addFileToFileList(sFileList* fl, sFile* f){
	sFile *walk, *last;

	if(fl->first == NULL){
		fl->first = f;
		fl->length++;
	}else{
		walk = fl->first;
		while(walk != NULL){
			if(walk->id != f->id){
				last = walk;
				walk = walk->next;
			}else{
				return;
			}
		}
		last->next = f;
		fl->length++;
	}
}

void printFileList(sFileList* fl){
	sFile		*fWalk;
	sClient		*cWalk;
	char buf[LIGNE_MAX];

	printd("printFileList : Début\n");
	fWalk = fl->first;
	while(fWalk != NULL){
		snprintf(buf,LIGNE_MAX,"{%d}\t%s\n",fWalk->id,fWalk->name);
		printd(buf);
		cWalk = fWalk->clients.first;
		while(cWalk != NULL){
			snprintf(buf,LIGNE_MAX,"\t\t%s\n",stringIP(cWalk->IP));
			printd(buf);
			cWalk = cWalk->next;
		}
		fWalk = fWalk->next;
	}
	printd("printFileList : Fin\n");
}

void printFile(sFile* f){
	char buf[LIGNE_MAX];
	snprintf(buf,LIGNE_MAX,"{%d}\t%s\n",f->id,f->name);
	printd(buf);
}

void printClient(sClient* f){
	char buf[LIGNE_MAX];
	snprintf(buf,LIGNE_MAX,"%s\n",stringIP(f->IP));
	printd(buf);
}

//Ajout des fichiers possédés par le client dans la liste de fichiers
int announceFiles(sFileList *fl, sFileTab clientFiles, unsigned int clientIP)
{
	//Déclarations
	int i;
	sFile *f;
	
	//Parcours du tableau des fichiers à ajouter
	for(i = 0; i < clientFiles.length; i++){
		f = fl->first;
		while (f != NULL)
		{
			//Si le fichier est déjà dans la liste, on ajoute uniquement l'adresse IP du client
			if (!strcmp(clientFiles.tab[i].name, f->name))
			{
				addClient(&f->clients, clientIP);
				break;
			}
			f = f->next;
		}
		if(f == NULL){
			addFile(fl, clientFiles.tab[i], clientIP);
		}
	}
	
	return 0;
}

//Ajout d'un fichier au début de la liste
int addFile(sFileList *fl, sFile f, unsigned int clientIP)
{
	//Déclarations
	sFile *new;
	new = malloc(sizeof(sFile));
	
	new->id = f.id;
	strcpy(new->name, f.name);
	new->clients.length = 0;
	new->clients.first = NULL;
	addClient(&new->clients, clientIP);
	new->next = fl->first;
	fl->first = new;
	return 0;
}

//Ajout d'un client dans la liste de clients
int addClient(sClientList *clients, unsigned int clientIP)
{
	//Déclarations
	sClient *new, *walk;

	walk = clients->first;
	while(walk != NULL){ // Parcours de tous les clients de la liste
		if(walk->IP == clientIP) // Si le client est déjà présent, s'arrêter
			return 0;
		walk = walk->next; // Continuer le parcours
	}
	new = malloc(sizeof(sClient)); // Le client n'est pas dans la liste

	new->IP = clientIP;			// Pas besoin de plus d'initialisation
	new->next = clients->first; // Placer le client en tête de liste
	clients->first = new;
	
	clients->length++;
	return 0;
}

//Recherche d'un fichier
//Prend en argument la liste des fichiers, le nom du fichier recherché
//Si des fichiers correspondent, renvoie une liste de fichiers et resultats contient le nombre de fichiers correspondant
//Si le fichier n'est pas trouvé, renvoie NULL
sFileTab *searchFileList(sFileList *fl, char *search)
{
	//Déclarations
	sFile *walk; 			//Permet le parcours de la liste de fichiers
	sFileTab *result; 		//Structure à renvoyer
	size_t tabSize = 0;
	
	result = malloc(sizeof(sFileTab));
	result->length = 0;
	result->tab = NULL;
	walk = fl->first;

	while (walk != NULL)
	{
		if (strstr(walk->name, search) != NULL)
		{
			tabSize += sizeof(sFile);
			result->tab = realloc(result->tab, tabSize);
			strcpy(result->tab[result->length].name,walk->name);
			result->tab[result->length].id = walk->id;
			result->length++;
		}
		walk = walk->next;
	}
	return result;
}

void freeFileList(sFileList *fl){
	sFile *fWalk, *fNext;
	sClient *cWalk, *cNext;
	fWalk = fl->first;
	while(fWalk != NULL){
		cWalk = fWalk->clients.first;
		while(cWalk != NULL){
			cNext = cWalk->next;
			free(cWalk);
			fWalk->clients.length--;
			cWalk = cNext;
		}
		fNext = fWalk->next;
		free(fWalk);
		fl->length--;
		fWalk = fNext;
	}
}

void deleteIPFileList(sFileList *fl, unsigned int IP){
	sFile 		*fWalk;
	sClient 	*cWalk, *cNext, *cPrev;
	sClientList *cl;
	fWalk = fl->first;
	while(fWalk != NULL){ // On parcours tous les fichiers
		cl = &fWalk->clients;
		cWalk = cl->first;
		while(cWalk != NULL){ // On parcours tous les clients possédant le fichier
			cPrev = NULL;
			cNext = cWalk->next;	// Ptr vers le client suivant car on supprime p-ê le client

			if(cWalk->IP == IP){	// Si le client est celui que l'on veut supprimer
				if(cPrev != NULL)	// S'il y a un précédent client
					cPrev->next = cNext;	// Le next du prec devient next du courant
				else				// Il est premier client de la liste
					cl->first = cNext;	// Le premier client devient son suivant
				free(cWalk);	// On peut supprimer le courant
				cl->length--;
			}else{
				cPrev = cWalk;	// On avance
			}
			cWalk = cNext; // Client suivant
		}
		fWalk = fWalk->next; // Fichier suivant
	}
}

sFile* getFileById(sFileList *fl, int id){
	sFile *walk;

	walk = fl->first;
	while(walk != NULL){
		if(walk->id == id)
			return walk;
		walk = walk->next;
	}
	return walk;
}