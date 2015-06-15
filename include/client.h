#include <pse.h>
#include <filelist.h>
#include <ncurses.h>
#include <sys/time.h>
#include <dialog.h>
#include <openssl/md5.h>

#define	CHUNK_SIZE	8192

typedef struct clientArgs{
	int socket;
	int commandeType;
} clientArgs;

typedef struct commande{
	int type;
} commande;

void printMenu();
void processMenu(char keycode, int *nbSox, int *continuer);
int initServerConn(struct sockaddr_in **serverAddr);
int serverOpen();
int sendServerCmd(int cmd);
int requestServer(int cmd, void *res, size_t res_len);
int announceServer();
int searchServer(char* search);
int requestFileListFromServer();
int requestFile(char* fileID);
void *tUL(void *arg);
void *tDL(void *arg);
void printFiles();
void getCommand(int peer);
off_t fsize(const char *file);
int sendFileDetails(int peer);
int queueChunks(int peer, sChunksList* chunkQueue);
int connectClient(sClient *c);
int launchDL(sFile *file);
int sendCommand(int cmd, int peer);
int computeMD5(unsigned char *buf, int chunkNum, int fileID);
void printMD5(unsigned char md5[MD5_DIGEST_LENGTH]);