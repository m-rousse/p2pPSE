#include <filelist.h>

sFileList* initFileList(){
	sFileList* fl = (sFileList*) malloc(sizeof(sFileList));
	fl->first = NULL;
	return fl;
}

void restoreFileList(sFileList* fl){
	char buf[LIGNE_MAX];
	int id, len;
	int fd; // File descriptor

	printd("restoreFileList : Debut");
	fd = open(SAVEFILE,O_RDONLY);
	if(fd >= 0){
		while((len = lireLigne(fd, buf)) > 0){
			id = atoi(buf);
			len = lireLigne(fd, buf);
			if(len > 0){
				addFileToFileList(fl,createFile(buf, id));
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
				snprintf(idString, LIGNE_MAX, "%d", walk->id);
				ecrireLigne(fd, idString);
				ecrireLigne(fd, walk->name);
				walk = walk->next;
			}while(walk != NULL);
		}
	}
	printd("saveFileList : Fin");
}

sFile* createFile(char* name, int id){
	sFile* f;

	f = (sFile*) malloc(sizeof(sFile));
	strncpy(f->name, name, LIGNE_MAX);
	f->id = id;
	f->next = NULL;
	return f;
}

void addFileToFileList(sFileList* fl, sFile* f){
	sFile* walk;

	if(fl->first == NULL){
		fl->first = f;
	}else{
		walk = fl->first;
		while(walk->next != NULL)
			walk = walk->next;
		walk->next = f;
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
				f->clients.length++;
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
	new->clients.length = 1;
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
	while(walk != NULL){
		if(walk->IP == clientIP)
			return 0;
		walk = walk->next;
	}
	new = malloc(sizeof(sClient));
	
	new->IP = clientIP;
	new->next = clients->first;
	clients->first = new;
	
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
			cWalk = cNext;
		}
		fNext = fWalk->next;
		free(fWalk);
		fWalk = fNext;
	}
}

void deleteIPFileList(sFileList *fl, unsigned int IP){
	sFile *fWalk;
	sClient *cWalk, *cNext, *cPrev;
	fWalk = fl->first;
	cPrev = NULL;
	while(fWalk != NULL){
		cWalk = fWalk->clients.first;
		while(cWalk != NULL){
			cNext = cWalk->next;
			if(cWalk->IP == IP){
				if(cPrev != NULL)
					cPrev->next = cNext;
				else
					fWalk->clients.first = cNext;
				free(cWalk);
				fWalk->clients.length--;
			}else{
				cPrev = cWalk;
			}
			cWalk = cNext;
		}
		fWalk = fWalk->next;
	}
}