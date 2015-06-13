#include <pse.h>

#define SAVEFILE "./downloads.txt"

typedef struct sClients{
	int 			IP; 			//adresse IP du client
	struct sClients *next; 	//client suivant dans la liste
} sClients;

typedef struct sClientsList{
	int 			length;	// Number of clients in list
	sClients 		*first; // First client of the list
} sClientsList;

typedef struct sFile {
	int 			id;					// Identifier of the file
	char 			name[LIGNE_MAX];	// Name of the file (on filesystem)
	struct sFile 	*next;				// Next file in case of a list of files
	sClientsList	clients;			// Clients who have the file
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


void sendFileList();
sFileList* initFileList();
void restoreFileList(sFileList* fl);
void saveFileList(sFileList* fl);
sFile* createFile(char* name, int id);
void addFileToFileList(sFileList* fl, sFile* f);
void printFileList(sFileList* fl);
void printFile(sFile *f);
void freeFileList(sFileList* fl);
int announceFiles(sFileList *fl, sFileTab clientFiles, int clientIP);
int addFile(sFileList *fl, sFile f, int clientIP);
int addClient(sClientsList *clients, int clientIP);