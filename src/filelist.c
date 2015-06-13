#include <filelist.h>

void sendFileList(){
	printd("FileList : Liste envoyée au serveur !");
}

sFileList* initFileList(){
	sFileList* fl = (sFileList*) malloc(sizeof(sFileList));
	fl->first = NULL;
	return fl;
}

void freeFileList(sFileList* fl){
	sFile *walk, *next;

	if(fl->first != NULL){
		walk = fl->first;
		do{
			next = walk->next;
			free(walk);
			walk = next;
		}while(walk != NULL);
	}
	free(fl);
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
	sFile* walk;
	char buf[LIGNE_MAX];
	printd("printFileList : Début\n");
	if(fl->first != NULL){
		walk = fl->first;
		do{
			snprintf(buf,LIGNE_MAX,"{%d}\t%s\n",walk->id,walk->name);
			printd(buf);
			walk = walk->next;
		}while(walk != NULL);
	}
	printd("printFileList : Fin\n");
}

void printFile(sFile* f){
	char buf[LIGNE_MAX];
	snprintf(buf,LIGNE_MAX,"{%d}\t%s\n",f->id,f->name);
	printd(buf);
}

//Ajout des fichiers possédés par le client dans la liste de fichiers
int announceFiles(sFileList *fl, sFileTab clientFiles, int clientIP)
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
int addFile(sFileList *fl, sFile f, int clientIP)
{
	//Déclarations
	sFile *new;
	new = malloc(sizeof(sFile));
	
	new->id = f.id;
	strcpy(new->name, f.name);
	new->clients.length = 1;
	addClient(&new->clients, clientIP);
	new->next = fl->first;
	fl->first = new;
	return 0;
}

//Ajout d'un client dans la liste de clients
int addClient(sClientsList *clients, int clientIP)
{
	//Déclarations
	sClients *new;
	new = malloc(sizeof(sClients));
	
	new->IP = clientIP;
	new->next = clients->first;
	clients->first = new;
	
	return 0;
}