#include <pse.h>

#define SAVEFILE "./downloads.txt"

typedef struct sClient{
	unsigned int 	IP; 	//adresse IP du client
	struct sClient 	*next; 	//client suivant dans la liste
} sClient;

typedef struct sClientList{
	int 			length;	// Number of clients in list
	sClient 		*first; // First client of the list
} sClientList;

// Structure quasi identique à la précédente, sauf que les éléments se suivent dans la mémoire
typedef struct sClientTab {
	int 			length;	// Number of files in the list
	sClient* 		tab;	// First element of the list
} sClientTab;


typedef struct sFile {
	int 			id;					// Identifier of the file
	char 			name[LIGNE_MAX];	// Name of the file (on filesystem)
	struct sFile 	*next;				// Next file in case of a list of files
	sClientList		clients;			// Clients who have the file
	int 			exists;				// If the file exists by the client
} sFile;

typedef struct sFileList {
	int 			length;	// Number of files in the list
	sFile* 			first;	// First element of the list
} sFileList;

// Structure quasi identique à la précédente, sauf que les éléments se suivent dans la mémoire
typedef struct sFileTab {
	int 			length;	// Number of files in the list
	sFile* 			tab;	// First element of the list
} sFileTab;


sFileList* initFileList();
void restoreFileList(sFileList* fl);
void saveFileList(sFileList* fl);
sFile* createFile(char* name, int id);
void addFileToFileList(sFileList* fl, sFile* f);
void printFileList(sFileList* fl);
void printFile(sFile *f);
void printClient(sClient *f);
void freeFileList(sFileList* fl);
int announceFiles(sFileList *fl, sFileTab clientFiles, unsigned int clientIP);
int addFile(sFileList *fl, sFile f, unsigned int clientIP);
int addClient(sClientList *clients, unsigned int clientIP);
sFileTab *searchFileList(sFileList *fl, char *search);
void deleteIPFileList(sFileList *fl, unsigned int IP);
void initFile(sFile *f);
void initClient(sClient *c);
void initClientList(sClientList *cl);
sFile* getFileById(sFileList *fl, int id);