#include <client.h>

int main(){
	sFileList* fileList;
	int continuer, val, listenSocket, ret, peers[10], nbSox, soxMax, i, n;
	socklen_t clientAddrLen;
	char menu;
	struct timeval timer, start;
	long microsec, delta;
	struct sockaddr_in listenAddr, clientAddr;
	fd_set peersSet;
	threadList tList;

	// Initialisation des variables
	nbSox = 0;
	soxMax = 0;
	memset(peers, 0, sizeof(peers));
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
		peers[nbSox] = accept(listenSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
		if(peers[nbSox] >= 0){
			if(peers[nbSox] > soxMax)
				soxMax = peers[nbSox];
			nbSox++;
			addThread(&tList, createThread());
			pthread_create(tList.last->thread, NULL, clientRequest, (void*) tList.last->args);
		}
		FD_ZERO(&peersSet);
		for(i = 0; i < nbSox; i++)
			FD_SET(peers[i], &peersSet);

		n = select(soxMax + 1, &peersSet, NULL, NULL, NULL);

		// Récupération d'une éventuelle commande
		menu = getch();
		// Calcul du temps depuis le dernier refresh
		gettimeofday(&timer, NULL);
		delta = timer.tv_sec*1000000+timer.tv_usec-microsec;
		if(delta > 200000){
			mvprintw(LINES-1,0,"Temps d'execution : %d \t\t\tNombre de clients : %d", timer.tv_sec - start.tv_sec, nbSox);
			val++;
			microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
		}
		// Si un caractère a été tapé, effacer la moitié basse de l'écran (pour le prochain message)
		if(menu != ERR){
			clrtobot();
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
	clientArgs *a = (clientArgs *)args;
	printf("Nouveau thread : %d\n",a->socket);
	pthread_exit(0);
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

void addThread(threadList* list, thread_t *t){
	if(list->first == NULL){
		list->first = list->last = t;
	}else{
		list->last->next = t;
		t->prev = list->last;
		list->last = t;
	}
}

void removeThread(threadList* list, thread_t *t){
	if(t->prev == NULL && t-> next == NULL){
		list->first = list->last = NULL;
	}else if(t-> prev == NULL){
		list->first = t->next;
	}else if(t->next == NULL){
		list->last = t->prev;
	}else{
		t->prev->next = t->next;
		t->next->prev = t->prev;
	}
}

void printThreads(threadList* list){
	thread_t *walk = list->first;
	while(walk != NULL){
		printd("Client thread : %d\n",walk->thread);
		walk = walk->next;
	}
}