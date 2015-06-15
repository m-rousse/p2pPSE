#include <pse.h>
#include <openssl/md5.h>

#define SAVEFILE "./downloads.txt"
#define	CHUNK_SIZE	8192
#define SRV_ADDR	"172.16.144.238"
#define SRV_PORT	"24240"
#define CLI_PORT	"24241"
#define UDP_PORT	"24242"

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

typedef struct sFileDetails {
	int 			id;					// Identifier of the file
	int 			size;				// Size of the file
	int 			nbChunks;			// Number of chunks that compose the file
} sFileDetails;

typedef struct sChunks {
	int 			fileID;					// Identifier of the file
	int 			num;					// Number of the chunk
	unsigned char  	hash[MD5_DIGEST_LENGTH];// Hash of the chunk
	sClient 		client;
	struct sChunks	*next;
} sChunks;

typedef struct sChunksList {
	int 			length;	// Number of chunks in the list
	sChunks* 		first;	// First element of the list
} sChunksList;

typedef struct sChunksTab {
	int 			length;	// Number of chunks in the list
	sChunks* 		tab;	// First element of the list
} sChunksTab;

typedef struct sData {
	int 			fileID;					// Identifier of the file
	int 			num;					// Number of the chunk
	char			data[CHUNK_SIZE];
} sData;

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
int addToChunkQueue(sChunksList *cl, sChunks *c);
int addToChunkTab(sChunksTab *ct, sChunks *c);
void removeChunk(sChunksList *cl, sChunks *c);