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
			free(walk->name);
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
	f->name = malloc(strlen(name)*sizeof(char));
	strncpy(f->name, name, strlen(name)*sizeof(char));
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