#include <pse.h>

#define SAVEFILE "./downloads.txt"
typedef struct sFile {
	int 			id;		// Identifier of the file
	char* 			name;	// Name of the file (on filesystem)
	struct sFile* 	next;	// Next file in case of a list of files
} sFile;

typedef struct sFileList {
	int 			length;	// Number of files in the list
	sFile* 			first;	// First element of the list
} sFileList;

void sendFileList();
sFileList* initFileList();
void restoreFileList(sFileList* fl);
void saveFileList(sFileList* fl);
sFile* createFile(char* name, int id);
void addFileToFileList(sFileList* fl, sFile* f);
void printFileList(sFileList* fl);