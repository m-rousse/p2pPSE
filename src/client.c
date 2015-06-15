#include <client.h>

sFileList 		*fileList;
sChunksTab		tmpQueue;
pthread_mutex_t mtxTmpQueue = PTHREAD_MUTEX_INITIALIZER;
sChunksList		inQueue;
pthread_mutex_t mtxInQueue = PTHREAD_MUTEX_INITIALIZER;
sChunksList		outQueue;
pthread_mutex_t mtxOutQueue = PTHREAD_MUTEX_INITIALIZER;

int main(){
	int 		continuer;
	int 		val;
	int 		listenSocket;
	int 		ret;
	int 		tmpPeer;
	int 		nbSox;
	int 		clearScr;
	socklen_t 	clientAddrLen;
	char 		menu;
	struct 		timeval timer, start;
	long 		microsec, delta;
	struct 		sockaddr_in listenAddr, clientAddr, *serverAddr;
	pthread_t 	threadUL;
	pthread_t 	threadDL;
	DataSpec	dataUL;
	DataSpec	dataDL;

	// Initialisation des variables
	nbSox = 0;
	clearScr = 0;
	clientAddrLen 	= sizeof(clientAddr);
	tmpQueue.tab 	= NULL;
	tmpQueue.length = 0;
	inQueue.first 	= NULL;
	inQueue.length 	= 0;
	outQueue.first 	= NULL;
	outQueue.length = 0;
	dataUL.quit 	= FAUX;
	dataDL.quit 	= FAUX;

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
	pthread_create(&threadUL, NULL, tUL, &dataUL);
	printd("Lancement du thread de DL");
	pthread_create(&threadDL, NULL, tDL, &dataDL);

	// ##################### ECOUTE #####################
	printd("Ouverture du socket d'écoute");
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(listenSocket < 0)
		erreur_IO("socket");

	listenAddr.sin_family = AF_INET;
	listenAddr.sin_addr.s_addr = INADDR_ANY;
	listenAddr.sin_port = htons(atoi(CLI_PORT));

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
			getCommand(tmpPeer);
			nbSox++;
		}

		// Récupération d'une éventuelle commande
		menu = getch();

		// Calcul du temps depuis le dernier refresh + Affichage
		gettimeofday(&timer, NULL);
		delta = timer.tv_sec*1000000+timer.tv_usec-microsec;
		if(delta > 200000){
			if(clearScr > 25){
				printMenu();
				move(21,0);
				clearScr = 0;
			}
			printFiles();
			mvprintw(LINES-1,0,"Temps d'execution : %d \t\t\tNombre de clients : %d", timer.tv_sec - start.tv_sec, nbSox);
			val++;
			clearScr++;
			microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
		}

		// Si un caractère a été tapé, nettoyer l'écran
		if(menu != ERR){
			clearScr = 25;
			mvprintw(LINES-1,0,"Temps d'execution : %d", timer.tv_sec - start.tv_sec);
			refresh();
		}
		move(21,0);

		// Traitement de la commande
		processMenu(menu, &nbSox, &continuer);
	}
	
	dataUL.quit = 1;
	pthread_join(threadUL, NULL);
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
	printw("	\n");
	printw("	============== Légende ==============\n");
	printw("	E : Fichier présent sur le disque\n");
	printw("	P : Nombre de peers possédant le fichier\n");
	printw("	D : Téléchargement du fichier en cours\n");
	//attroff(A_BOLD | A_UNDERLINE);
}

void processMenu(char keycode, int *nbSox, int *continuer){
	int ret;
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
			break;
		case '5':
			printFileList(fileList);
			break;
		case '6':
			ret = dialog_msgbox("Se connecter a un client", "Commande supprimee", 10, 30, 1);
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

int initServerConn(struct sockaddr_in **serverAddr){
	*serverAddr = resolv(SRV_ADDR, SRV_PORT);
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
		//printFile(&fileTab.tab[i]);
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

	// ############ CLEAN FILES CLIENT TABLE ############

	//Parcours du tableau des clients à connecter
	for(i = 0; i < clientTab.length; i++){
		addClient(&file->clients, clientTab.tab[i].IP);
	}
	if(recvSize == 0)
		printd("Aucun fichier n'a été trouvé.\n");
	else
		launchDL(file);
	close(sock);
	return 0;
}

void printFiles(){
	sFile 	*walk;
	char	buf[LIGNE_MAX];

	wprintw(stdscr,"\t========== Téléchargements ==========\n");
	wprintw(stdscr,"\tThreadUL %d\t-\tThreadDL %d\n", outQueue.length, inQueue.length);
	wprintw(stdscr,"\t");
	attron(A_BOLD | A_UNDERLINE);
	wprintw(stdscr,"fileID\tE\tP\tD\t");
	attroff(A_BOLD | A_UNDERLINE);
	wprintw(stdscr,"\n");
	walk = fileList->first;
	while(walk != NULL){
		snprintf(buf, LIGNE_MAX, "\t%6d\t%d\t%d\t0\t\n", walk->id, walk->exists, walk->clients.length);
		wprintw(stdscr,buf);
		walk = walk->next;
	}
}

void *tUL(void *arg)
{
	//Déclarations
	DataSpec 	*data = (DataSpec *) arg;
	struct sockaddr_in 	servAddr;
	int 				fd;
	sData				*buf;
	struct hostent 		*hp;
	sChunks 			*walk;
	sFile 				*file;
	FILE 				*in;
	int 				bytes;

	while(!data->quit){
		usleep(500);
		if(outQueue.length > 0){
			walk = outQueue.first;

			fd = socket(AF_INET, SOCK_DGRAM, 0);
			if(fd < 0)
				pthread_exit(NULL);

			memset((char*)&servAddr, 0, sizeof(servAddr));
			servAddr.sin_family = AF_INET;
			servAddr.sin_port = htons(atoi(UDP_PORT));

			hp = gethostbyname(stringIP(ntohl(walk->client.IP)));
			if(!hp){
				close(fd);
				pthread_exit(NULL);
			}
			memcpy((void *)&servAddr.sin_addr, hp->h_addr_list[0], hp->h_length);

			buf = malloc(sizeof(sData));
			if(buf == NULL){
				close(fd);
				pthread_exit(NULL);
			}
			buf->fileID = walk->fileID;
			buf->num = walk->num;

			file = getFileById(fileList, walk->fileID);
			if(file == NULL){
				close(fd);
				pthread_exit(NULL);
			}
			in = fopen(file->name, "rb");
			if(in == NULL){
				close(fd);
				pthread_exit(NULL);
			}
			fseek(in, walk->num*CHUNK_SIZE, SEEK_SET);
			bytes = fread(buf->data, CHUNK_SIZE, 1, in);
			if(bytes < 0){
				fclose(in);
				close(fd);
				pthread_exit(NULL);
			}
			sendto(fd, buf, sizeof(buf), 0, (struct sockaddr *) &servAddr, sizeof(servAddr));
			pthread_mutex_lock(&mtxOutQueue);
			removeChunk(&outQueue, walk);
			pthread_mutex_unlock(&mtxOutQueue);
			fclose(in);
			close(fd);
			free(buf);
		}
	}
	printf("C'est la fin pour moi ! (%d)\n", (int) data->id);
	pthread_exit(NULL);
}

void *tDL(void *arg)
{
	//Déclarations
	DataSpec *data = (DataSpec *) arg;
	struct sockaddr_in 	listAddr;
	struct sockaddr_in 	cliAddr;
	socklen_t 			addrLen = sizeof(cliAddr);
	int 				recvlen;
	int 				fd;
	sData				*buf;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		pthread_exit(NULL);
	}

	memset((char *)&listAddr, 0, sizeof(listAddr));
	listAddr.sin_family = AF_INET;
	listAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listAddr.sin_port = htons(atoi(UDP_PORT));

	if(bind(fd, (struct sockaddr *) &listAddr, sizeof(listAddr)) < 0)
		pthread_exit(NULL);

	buf = malloc(sizeof(sData));
	memset(buf, 0, sizeof(sData));

	while(!data->quit){
		usleep(500);
		if(inQueue.length > 0){
			recvlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *) &cliAddr, &addrLen);
			if(recvlen > 0){
				processIncoming(&inQueue, buf);
			}
		}
	}
	printf("C'est la fin pour moi ! (%d)\n", (int) data->id);
	free(buf);
	close(fd);
	pthread_exit(NULL);
}

void getCommand(int peer){
	// 1 - demander infos sur fichier
	// 2 - demander des morceaux
	int 	cmd;

	cmd = -1;
	read(peer, &cmd, sizeof(int));

	switch(cmd){
		case 1:
			sendFileDetails(peer);
			break;
		case 2:
			pthread_mutex_lock(&mtxOutQueue);
			queueChunks(peer, &outQueue);
			pthread_mutex_unlock(&mtxOutQueue);
			break;
		default:
			break;
	}
	close(peer);
}

void processIncoming(sChunksList *cl, sData *d){
	sChunks 	*walk;
	sFile 		*file;
	FILE 		*f;

	walk = cl->first;
	if(d == NULL)
		return;
	while(walk != NULL){
		if(d->fileID == walk->fileID && d->num == walk->num){
			// Ajouter le contenu du paquet au fichier
			file = getFileById(fileList,walk->fileID);
			if(file == NULL)
				;
			f = fopen(file->name, "wb+");
			fwrite(d->data, CHUNK_SIZE, 1, f);
			fclose(f);
			removeChunk(cl, walk);
		}
		walk = walk->next;
	}
}

int launchDL(sFile *file){
	sClient 		*cWalk;
	int 			tmpPeer;
	int 			numBytes;
	int 			cmd;
	int 			i;
	int 			k;
	int 			numChunks;
	sFileDetails 	*fd;
	sChunks 		*chunk;

	cWalk = file->clients.first;

	if(cWalk != NULL){
		tmpPeer = connectClient(cWalk);
		cmd = 1;
		numBytes = write(tmpPeer, &cmd, sizeof(int));
		if(numBytes < 0)
			return -1;
		numBytes = write(tmpPeer, &file->id, sizeof(int));
		if(numBytes < 0)
			return -1;

		fd = malloc(sizeof(sFileDetails));
		numBytes = read(tmpPeer, fd, sizeof(sFileDetails));
		if(numBytes < 0)
			return -1;
		close(tmpPeer);
	}

	i = 0;
	while(cWalk != NULL){
		// Placer dans le buffer d'entrée tous les chunks à demander
		// PUIS faire la demande d'envoi

		// Parcours du nombre de chunks pour répartition selon les clients
		for(k = i; k < fd->nbChunks; k += file->clients.length){
			chunk = malloc(sizeof(sChunks));
			chunk->fileID = file->id;
			chunk->num = k;
			memset(chunk->hash, 0, MD5_DIGEST_LENGTH);
			chunk->next = NULL;
			chunk->client.IP = cWalk->IP;
			chunk->client.next = NULL;
			pthread_mutex_lock(&mtxTmpQueue);
			addToChunkTab(&tmpQueue, chunk);
			pthread_mutex_unlock(&mtxTmpQueue);
		}
		tmpPeer = connectClient(cWalk);
		cmd = 2;
		numBytes = write(tmpPeer, &cmd, sizeof(int));
		if(numBytes < 0)
			return -1;
		numBytes = write(tmpPeer, &tmpQueue.length, sizeof(int));
		if(numBytes < 0)
			return -1;
		pthread_mutex_lock(&mtxTmpQueue);
		numChunks = tmpQueue.length;
		numBytes = write(tmpPeer, tmpQueue.tab, numChunks*sizeof(sChunks));
		if(numBytes < 0)
			return -1;
		numBytes = read(tmpPeer, tmpQueue.tab, numChunks*sizeof(sChunks));
		if(numBytes < 0)
			return -1;
		for(i = 0; i < numChunks; i++){
			chunk = malloc(sizeof(sChunks));
			chunk->fileID = tmpQueue.tab[i].fileID;
			chunk->num = tmpQueue.tab[i].num;
			memcpy(chunk->hash, tmpQueue.tab[i].hash, MD5_DIGEST_LENGTH);
			chunk->next = NULL;
			pthread_mutex_lock(&mtxInQueue);
			addToChunkQueue(&inQueue, chunk);
			pthread_mutex_unlock(&mtxInQueue);
		}
		cWalk = cWalk->next;
		pthread_mutex_unlock(&mtxTmpQueue);
	}
	return 0;
}

int sendCommand(int cmd, int peer){
	write(peer, &cmd, sizeof(int));
	return 0;
}

int connectClient(sClient *c){
	int 				sock;
	int 				ret;
	struct sockaddr_in	*sa;

	if(c == NULL)
		return -1;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		return -1;
	sa = resolv(stringIP(c->IP), CLI_PORT);
	if(sa == NULL)
		return -1;
	ret = connect(sock, (struct sockaddr *) sa, sizeof(struct sockaddr_in));
	if(ret < 0)
		return -1;
	freeResolv();
	return sock;
}

int sendFileDetails(int peer){
	int 			fileID;
	int 			numBytes;
	sFileDetails 	fDetails;
	sFile 			*file;

	fileID = -1;
	numBytes = read(peer, &fileID, sizeof(int));
	if(numBytes < 0)
		return -1;
	file = getFileById(fileList, fileID);
	if(file == NULL)
		return -1;
	
	fDetails.size = fsize(file->name);
	if(fDetails.size < 0)
		return -1;
	
	fDetails.id = fileID;
	fDetails.nbChunks = fDetails.size / CHUNK_SIZE + 1;

	numBytes = write(peer, &fDetails, sizeof(sFileDetails));
	if(numBytes < 0)
		return -1;
	return 0;
}

off_t fsize(const char *file){
    struct stat st; 

    if (stat(file, &st) == 0)
        return st.st_size;

    return -1; 
}

// Recevoir + placer dans la chunkQueue des chunks
int queueChunks(int peer, sChunksList* chunkQueue){
	// mtxChunkQueue
	sChunksTab	tab;
	int 		numChunks;
	int 		numBytes;
	int 		i;
	sChunks 	*tmp;
	char		blank[MD5_DIGEST_LENGTH];
	struct sockaddr_in peerAddr;
	socklen_t	peerLen;

	numBytes = read(peer, &numChunks, sizeof(int));
	if(numBytes < 0)
		return -1;
	tab.length = numChunks;
	tab.tab = malloc(numChunks*sizeof(sChunks));
	if(tab.tab == NULL)
		return -1;
	numBytes = read(peer, tab.tab, sizeof(sChunks)*numChunks);
	if(numBytes < 0)
		return -1;

	peerLen = sizeof(peerAddr);
	getpeername(peer, (struct sockaddr *) &peerAddr, &peerLen);

	memset(&blank, 0, MD5_DIGEST_LENGTH);
	for(i = 0; i < numChunks; i++){
		tmp = malloc(sizeof(sChunks));
		tmp->fileID = tab.tab[i].fileID;
		tmp->num = tab.tab[i].num;
		memcpy(tmp->hash, tab.tab[i].hash, MD5_DIGEST_LENGTH);
		if(memcmp(tmp->hash,blank,MD5_DIGEST_LENGTH)){
			computeMD5(tmp->hash, tmp->num, tmp->fileID);
			memcpy(tab.tab[i].hash, tmp->hash, MD5_DIGEST_LENGTH);
		}
		tmp->client.IP = peerAddr.sin_addr.s_addr;
		tmp->next = NULL;
		addToChunkQueue(chunkQueue, tmp);
	}
	numBytes = write(peer, tab.tab, sizeof(sChunks)*numChunks);
	if(numBytes < 0)
		return -1;

	free(tab.tab);
	return 0;
}

int sendChunkDetails(int peer){
	return 0;
}

int computeMD5(unsigned char buf[MD5_DIGEST_LENGTH], int chunkNum, int fileID){
	FILE 			*in;
	sFile 			*f;
	MD5_CTX 		mC;
	int 			bytes;
	unsigned char	data[CHUNK_SIZE];

	f = getFileById(fileList, fileID);
	if(f == NULL)
		return -1;
	in = fopen(f->name, "rb");
	if(in == NULL)
		return -1;
	MD5_Init(&mC);
	fseek(in, chunkNum*CHUNK_SIZE, SEEK_SET);
	bytes = fread(data, CHUNK_SIZE, 1, in);
	if(bytes < 0){
		fclose(in);
		return -1;
	}
	MD5_Update(&mC, data, bytes);
	MD5_Final(buf, &mC);
	return 0;
}

void printMD5(unsigned char md5[MD5_DIGEST_LENGTH]){
	int 	i;
	for(i = 0; i < MD5_DIGEST_LENGTH; i++)
		printf("%02x", md5[i]);
}