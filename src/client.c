#include <client.h>

int main(){
	sFileList* fileList;
	int continuer, val, listenSocket, ret, tmpPeer, nbSox, *threadRetVal, clearScr;
	socklen_t clientAddrLen;
	char menu;
	struct timeval timer, start;
	long microsec, delta;
	struct sockaddr_in listenAddr, clientAddr;
	threadList tList;
	thread_t *walk;

	// Initialisation des variables
	nbSox = 0;
	clearScr = 0;
	threadRetVal = (int*) malloc(sizeof(int));
	tList.first = NULL;
	tList.last  = NULL;

	// Initialisation de NCurses (init, getch non bloquants, pas d'affichage des caractères tapés)
	initscr();
	timeout(0);
	noecho();
	printd("Démarrage");

	// Initialisation de la liste des fichiers possédés
	fileList = initFileList();
	restoreFileList(fileList);

	printd("Annonce des fichiers possédés");
	sendFileList();

	printd("Lancement du thread d'UL");
	printd("Lancement du thread de DL");

	printd("Ouverture du socket d'écoute");
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(listenSocket < 0)
		erreur_IO("socket");

	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = INADDR_ANY;
	listenAddr.sin_port = htons(24241);

	ret = bind(listenSocket, (struct sockaddr *) &listenAddr, sizeof(listenAddr));
	if(ret < 0)
		erreur_IO("bind");

	ret = fcntl (listenSocket, F_GETFL, 0);
    fcntl (listenSocket, F_SETFL, ret | O_NONBLOCK);

	ret = listen(listenSocket, 10);
	if(ret < 0)
		erreur_IO("listen");

	printd("Affichage des infos du client + menu");

	refresh();
	continuer = 1;
	val = 0;
	gettimeofday(&timer, NULL);
	gettimeofday(&start, NULL);
	microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
	printMenu();

	while(continuer){
		// Acceptation des nouveaux clients
		clientAddrLen = sizeof(clientAddr);
		tmpPeer = accept(listenSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
		if(tmpPeer >= 0){
			addThread(&tList, createThread());
			tList.last->args->socket = tmpPeer;
			pthread_create(tList.last->thread, NULL, clientRequest, (void*) tList.last->args);
			nbSox++;
		}

		walk = tList.first;
		while(walk != NULL){
			if(walk->args->socket < 0){
				pthread_join(*(walk->thread), (void**) &threadRetVal);
				printd("Fin du thread, retour : %d", *threadRetVal);
				nbSox--;
				thread_t *tmp = walk;
				threadList *tmpList = &tList;
				walk = removeThread(&tmpList, walk);
				destroyThread(tmp);
			}else{
				walk = walk->next;
			}
		}

		// Récupération d'une éventuelle commande
		menu = getch();
		// Calcul du temps depuis le dernier refresh
		gettimeofday(&timer, NULL);
		delta = timer.tv_sec*1000000+timer.tv_usec-microsec;
		if(delta > 200000){
			if(clearScr > 5){
				clrtobot();
				clearScr = 0;
			}
			mvprintw(LINES-1,0,"Temps d'execution : %d \t\t\tNombre de clients : %d", timer.tv_sec - start.tv_sec, nbSox);
			val++;
			clearScr++;
			microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
		}
		// Si un caractère a été tapé, effacer la moitié basse de l'écran (pour le prochain message)
		if(menu != ERR){
			clrtobot();
			clearScr = 0;
			mvprintw(LINES-1,0,"Temps d'execution : %d", timer.tv_sec - start.tv_sec);
			refresh();
		}
		move(15,0);
		switch(menu){
			case '1':
				printd("Affichage des clients\n");
				printThreads(&tList);
				break;
			case '2':
				printFileList(fileList);
				break;
			case '3':
				printd("Connection\n");
				break;
			case '4':
				printd("Imuseless:)\n");
				break;
			case 'q':
				continuer = 0;
				break;
			default:
				break;
		}
	}
	
	printd("Sauvegarde de la liste des fichiers possédés");
	saveFileList(fileList);
	freeFileList(fileList);
	close(listenSocket);
	endwin();

	return 0;
}

void printMenu(){
	clear();
	//attron(A_BOLD | A_UNDERLINE);
	printw("==== Menu ====\n");
	printw("1 - Afficher les clients\n");
	printw("2 - Afficher les fichiers possédés\n");
	printw("3 - Se connecter à un client\n");
	printw("4 - Do stuff\n");
	printw("\n");
	printw("q - Quit\n");
	//attroff(A_BOLD | A_UNDERLINE);
}

void *clientRequest(void *args){
	int* ret;
	clientArgs *a;
	commande *comm;

	ret = (int*) malloc(sizeof(int));
	a = (clientArgs *)args;
	comm = (commande*) malloc(sizeof(commande));

	// Traitement des commandes
	comm->type = 1;
	write(a->socket, comm, sizeof(comm));

	// Fin du thread, fermeture du socket et annonce au thread main que le thread a fini
	close(a->socket);
	a->socket = -1;
	*ret = 42;
	pthread_exit(ret);
}

thread_t *createThread(){
	thread_t *t = (thread_t *) malloc(sizeof(thread_t));
	t->prev = NULL;
	t->next = NULL;
	t->thread = (pthread_t*) malloc(sizeof(pthread_t));

	// Initialisation des arguments du thread
	t->args = (clientArgs*) malloc(sizeof(clientArgs));
	t->args->socket = -1;
	return t;
}

void destroyThread(thread_t *t){
	if(t == NULL)
		return;
	free(t->args);
	free(t->thread);
	free(t);
}

void addThread(threadList* list, thread_t *t){
	if(list->first == NULL){
		list->first = list->last = t;
	}else{
		list->last->next = t;
		t->prev = list->last;
		list->last = t;
	}
}

thread_t *removeThread(threadList** list, thread_t *t){
	if(t == NULL){
		return NULL;
	}
	if(t->prev == NULL && t->next == NULL){
		(*list)->first = (*list)->last = NULL;
	}else if(t-> prev == NULL){
		(*list)->first = t->next;
		t->next->prev = NULL;
		return t->next;
	}else if(t->next == NULL){
		(*list)->last = t->prev;
		t->prev->next = NULL;
	}else{
		t->prev->next = t->next;
		t->next->prev = t->prev;
		return t->next;
	}
	return NULL;
}

void printThreads(threadList* list){
	thread_t *walk = list->first;
	while(walk != NULL){
		printd("Client thread : %d\n",walk->thread);
		walk = walk->next;
	}
}