#include <client.h>

sFileList* fileList;
int main(){
	int continuer, val, listenSocket, ret, tmpPeer, nbSox, *threadRetVal, clearScr;
	socklen_t clientAddrLen;
	char menu;
	struct timeval timer, start;
	long microsec, delta;
	struct sockaddr_in listenAddr, clientAddr, *serverAddr;
	threadList tList;
	thread_t *walk;

	// Initialisation des variables
	nbSox = 0;
	clearScr = 0;
	threadRetVal = (int*) malloc(sizeof(int));
	tList.first = NULL;
	tList.last  = NULL;
	clientAddrLen = sizeof(clientAddr);

	// Initialisation de NCurses (init, getch non bloquants, pas d'affichage des caractères tapés)
	initscr();
	//init_dialog(stdin, stdout);
	dialog_vars.ascii_lines = 1;
	timeout(0);
	noecho();
	printd("Démarrage");

	// Connection au serveur
	initServerConn(&serverAddr);

	// Initialisation de la liste des fichiers possédés
	fileList = initFileList();
	restoreFileList(fileList);

	printd("Annonce des fichiers possédés");
	announceServer();

	printd("Lancement du thread d'UL");
	printd("Lancement du thread de DL");

	// ##################### ECOUTE #####################
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


	// ############## MENU + TRAITEMENT ##############
	refresh();
	continuer = 1;
	val = 0;
	gettimeofday(&timer, NULL);
	gettimeofday(&start, NULL);
	microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
	printMenu();

	while(continuer){
		usleep(500);

		// Acceptation des nouveaux clients
		tmpPeer = accept(listenSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
		if(tmpPeer >= 0){
			addThread(&tList, createThread());
			tList.last->args->socket = tmpPeer;
			pthread_create(tList.last->thread, NULL, incomingClient, (void*) tList.last->args);
			nbSox++;
		}

		// Nettoyage des threads finis
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

		// Calcul du temps depuis le dernier refresh + Affichage
		gettimeofday(&timer, NULL);
		delta = timer.tv_sec*1000000+timer.tv_usec-microsec;
		if(delta > 200000){
			if(clearScr > 25){
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
		move(17,0);

		// Traitement de la commande
		processMenu(menu, &tList, &nbSox, &continuer);
	}
	
	printd("Sauvegarde de la liste des fichiers possédés");
	saveFileList(fileList);
	freeFileList(fileList);
	close(listenSocket);
	//end_dialog();
	endwin();
	return 0;
}

void printMenu(){
	clear();
	//attron(A_BOLD | A_UNDERLINE);
	printw("	\n");
	printw("	================ Menu ================\n");
	printw("	1 - Rechercher un fichier\n");
	printw("	2 - Afficher les fichiers disponibles\n");
	printw("	9 - Télécharger un fichier\n");
	printw("	\n");
	printw("	-----> Debug\n");
	printw("	3 - Annoncer les fichiers possédés\n");
	printw("	4 - Afficher les peers\n");
	printw("	5 - Afficher les fichiers possédés\n");
	printw("	6 - Se connecter à un client\n");
	printw("	7 - Arreter le serveur\n");
	printw("	p - Afficher les peers d'un fichier\n");
	printw("	\n");
	printw("	q - Quit\n");
	//attroff(A_BOLD | A_UNDERLINE);
}

void processMenu(char keycode, threadList *tList, int *nbSox, int *continuer){
	int ret, tmpPeer;
	struct sockaddr_in *remoteAddr;
	sFile *affClientsF;
	sClient *affClientsC;

	switch(keycode){
		case '1':
			ret = dialog_inputbox("Recherche", "Entrez votre recherche", 10, 30, "", 0);
			clrtobot();
			if(ret == 0){
				searchServer( dialog_vars.input_result);
			}else{
				ret = dialog_msgbox("Recherche", "\nErreur à la saisie", 10, 30, 0);
			}
			break;
		case '2':
			requestFileListFromServer();
			break;
		case '3':
			announceServer();
			break;
		case '4':
			printd("Affichage des clients\n");
			printThreads(tList);
			break;
		case '5':
			printFileList(fileList);
			break;
		case '6':
			ret = dialog_inputbox("Se connecter a un client", "Entrez l'adresse IP", 10, 30, "", 0);
			clrtobot();

			if(ret == 0){
				// Connection au client
				tmpPeer = socket(AF_INET, SOCK_STREAM, 0);
				if(tmpPeer >= 0){
					remoteAddr = resolv(dialog_vars.input_result,"24241");
					if(remoteAddr != NULL){
						fcntl(tmpPeer, F_SETFL, O_NONBLOCK);
						ret = connect(tmpPeer, (struct sockaddr *) remoteAddr, sizeof(struct sockaddr));
						if(ret < 0)
							;// ERREUR
						freeResolv();

						addThread(tList, createThread());
						tList->last->args->socket = tmpPeer;
						ret = dialog_inputbox("Se connecter a un client", "Commande", 10, 30, "", 0);
						if(ret >= 0)
							tList->last->args->commandeType = atoi(dialog_vars.input_result);
						pthread_create(tList->last->thread, NULL, outgoingClient, (void*) tList->last->args);
						(*nbSox)++;
					}else{
						printd("Adresse erronee (resolv)");
					}
				}else{
					erreur_IO("socket");
				}
			}
			break;
		case '7':
			printw("Say goobye to your server 3:)");
			sendServerCmd(7);
			break;
		case '9':
			ret = dialog_inputbox("Telecharger un fichier", "Entrez le fileID", 10, 30, "", 0);
			clrtobot();

			if(ret == 0){
				requestFile( dialog_vars.input_result);
			}else{
				ret = dialog_msgbox("Telecharger un fichier", "\nErreur à la saisie", 10, 30, 0);
			}
			break;
		case 'p':
			ret = dialog_inputbox("Clients d'un fichier", "Entrez le fileID", 10, 30, "", 0);
			clrtobot();

			if(ret == 0){
				affClientsF = getFileById(fileList, atoi(dialog_vars.input_result));
				if(affClientsF == NULL){
					ret = dialog_msgbox("Clients d'un fichier !","FileID non trouvé !", 10, 30, 0);
					break;
				}
				affClientsC = affClientsF->clients.first;
				while(affClientsC != NULL){
					printClient(affClientsC);
					affClientsC = affClientsC->next;
				}
			}else{
				ret = dialog_msgbox("Clients d'un fichier", "\nErreur à la saisie", 10, 30, 0);
			}
			break;
		case 'q':
			*continuer = 0;
			printw("ByeBye :)");
			sendServerCmd(8);
			break;
		default:
			break;
	}
}

// Gestion des connections provenant d'autres clients
void *incomingClient(void *args){
	int* ret;
	clientArgs *a;
	commande *comm;

	ret = (int*) malloc(sizeof(int));
	a = (clientArgs *)args;
	comm = (commande*) malloc(sizeof(commande));

	// Traitement des commandes
	recv(a->socket, comm, sizeof(comm), 0);

	switch(comm->type){
		case 1:
			move(10,0);
			printd("Demande d'UL\n");
			break;
		default:
			break;
	}

	// Fin du thread, fermeture du socket et annonce au thread main que le thread a fini
	close(a->socket);
	a->socket = -1;
	*ret = comm->type;
	pthread_exit(ret);
}

// Gestion des connections à destination d'autres clients
void *outgoingClient(void *args){
	int* ret;
	clientArgs *a;
	commande *comm;

	ret = (int*) malloc(sizeof(int));
	a = (clientArgs *)args;
	comm = (commande*) malloc(sizeof(commande));

	// Traitement des commandes
	comm->type = a->commandeType;
	send(a->socket, comm, sizeof(comm), 0);

	// Fin du thread, fermeture du socket et annonce au thread main que le thread a fini
	close(a->socket);
	a->socket = -1;
	*ret = comm->type;
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

int initServerConn(struct sockaddr_in **serverAddr){
	*serverAddr = resolv("172.16.144.228", "24240");
	if(*serverAddr == NULL)
		erreur("Resolution DNS impossible\n");
	freeResolv();
	return 0;
}

int serverOpen(){
	int serverSock, ret;
	struct sockaddr_in *serverAddr;

	initServerConn(&serverAddr);
	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSock < 0)
		return serverSock;
	ret = connect(serverSock, (struct sockaddr *) serverAddr, sizeof(struct sockaddr_in));
	if(ret < 0)
		return ret;
	return serverSock;
}

int sendServerCmd(int cmd){
	int sock;

	sock = serverOpen();
	if(sock < 0)
		return -1;
	send(sock, (void *) &cmd, sizeof(cmd), 0);
	close(sock);
	return 0;
}

int announceServer(){
	int sock, i, cmd;
	size_t flSize;
	sFile *buf, *walk;

	cmd = 3;
	i = 0;
	walk = fileList->first;
	while(walk != NULL){
		if(walk->exists)
			i++;
		walk = walk->next;
	}
	flSize = i*sizeof(sFile);
	buf = malloc(flSize);
	i = 0;
	walk = fileList->first;
	while(walk != NULL){
		if(walk->exists){
			memcpy((void *) &buf[i], walk, sizeof(sFile));
			i++;
		}
		walk = walk->next;
	}
	sock = serverOpen();
	send(sock, (void *) &cmd, sizeof(cmd), 0);
	send(sock, (void *) &i, sizeof(i),0);
	send(sock, (void *) buf, flSize, 0);
	close(sock);
	return 0;
}

int searchServer(char* search){
	int 		sock, i, cmd, recvSize;
	size_t 		filesSize;
	sFileTab 	fileTab;
	char		buf[LIGNE_MAX];

	// Envoi de la commande recherche + terme recherché
	cmd = 1;
	memset(buf, 0, LIGNE_MAX*sizeof(char));
	strcpy(buf, search);
	sock = serverOpen();
	send(sock, (void *) &cmd, sizeof(cmd), 0);
	send(sock, (void *) search, LIGNE_MAX*sizeof(char), 0);

	// Reception des résultats
	recv(sock, &recvSize, sizeof(recvSize), 0);
	filesSize = recvSize*sizeof(sFile);
	fileTab.length = recvSize;
	if(recvSize > 0){
		fileTab.tab = malloc(filesSize);
		recv(sock, fileTab.tab, filesSize, 0);
	}else{
		fileTab.tab = NULL;
	}

	//Parcours du tableau des fichiers à ajouter
	for(i = 0; i < fileTab.length; i++){
		printFile(&fileTab.tab[i]);
	}
	if(recvSize == 0)
		printd("Aucun fichier n'a été trouvé.\n");
	close(sock);
	return 0;
}

int requestFileListFromServer(){
	int 		sock, i, cmd, recvSize;
	size_t 		filesSize;
	sFileTab 	fileTab;
	sFile 		*tmp;

	// Envoi de la commande recherche
	cmd = 2;
	sock = serverOpen();
	send(sock, (void *) &cmd, sizeof(cmd), 0);

	// Reception des résultats
	recv(sock, &recvSize, sizeof(recvSize), 0);
	filesSize = recvSize*sizeof(sFile);
	fileTab.length = recvSize;
	if(recvSize > 0){
		fileTab.tab = malloc(filesSize);
		recv(sock, fileTab.tab, filesSize, 0);
	}else{
		fileTab.tab = NULL;
	}

	//Parcours du tableau des fichiers à afficher
	for(i = 0; i < fileTab.length; i++){
		tmp = malloc(sizeof(sFile));
		initFile(tmp);
		tmp->id = fileTab.tab[i].id;
		strcpy(tmp->name, fileTab.tab[i].name);
		addFileToFileList(fileList, tmp);
		printFile(&fileTab.tab[i]);
	}
	if(recvSize == 0)
		printd("Aucun fichier n'a été trouvé.\n");
	close(sock);
	return 0;
}

int requestFile(char* fileID){
	int 		sock, i, cmd, recvSize, iFileID;
	size_t 		clientsSize;
	sClientTab 	clientTab;
	sFile 		*file;

	// Envoi de la commande telechargement + fileID
	cmd = 9;
	iFileID = atoi(fileID);
	sock = serverOpen();
	send(sock, (void *) &cmd, sizeof(cmd), 0);
	send(sock, (void *) &iFileID, sizeof(int), 0);

	// Reception des résultats
	recv(sock, &recvSize, sizeof(recvSize), 0);
	clientsSize = recvSize*sizeof(sClient);
	clientTab.length = recvSize;
	if(recvSize > 0){
		clientTab.tab = malloc(clientsSize);
		recv(sock, clientTab.tab, clientsSize, 0);
	}else{
		clientTab.tab = NULL;
	}

	file = getFileById(fileList, iFileID);
	if(file == NULL){
		requestFileListFromServer();
		file = getFileById(fileList, iFileID);
		if(file == NULL)
			return -1;
	}

	//Parcours du tableau des clients à connecter
	for(i = 0; i < clientTab.length; i++){
		addClient(&file->clients, clientTab.tab[i].IP);
	}
	if(recvSize == 0)
		printd("Aucun fichier n'a été trouvé.\n");
	close(sock);
	return 0;
}