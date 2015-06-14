#include <pse.h>
#include <filelist.h>
#include <ncurses.h>
#include <sys/time.h>
#include <dialog.h>

typedef struct clientArgs{
	int socket;
	int commandeType;
} clientArgs;

typedef struct thread_t{
	pthread_t 		*thread;
	struct thread_t	*next;
	struct thread_t	*prev;
	clientArgs		*args;
} thread_t;

typedef struct threadList{
	thread_t *first;
	thread_t *last;
} threadList;

typedef struct commande{
	int type;
} commande;

void printMenu();
void processMenu(char keycode, threadList *tList, int *nbSox, int *continuer);
void *incomingClient(void *args);
void *outgoingClient(void *args);
thread_t *createThread();
void addThread(threadList* list, thread_t *t);
thread_t *removeThread(threadList** list, thread_t *t);
void printThreads(threadList* list);
void destroyThread(thread_t *t);
int initServerConn(struct sockaddr_in **serverAddr);
int serverOpen();
int sendServerCmd(int cmd);
int requestServer(int cmd, void *res, size_t res_len);
int announceServer();
int searchServer(char* search);
int requestFileListFromServer();
int requestFile(char* fileID);
void printFiles();