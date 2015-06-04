#include <pse.h>
#include <filelist.h>
#include <ncurses.h>
#include <sys/time.h>

typedef struct clientArgs{
	int socket;
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

void printMenu();
void *clientRequest(void *args);
thread_t *createThread();
void addThread(threadList* list, thread_t *t);
void removeThread(threadList* list, thread_t *t);
void printThreads(threadList* list);