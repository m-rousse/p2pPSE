#include <client.h>

int main(){
	sFileList* fileList;
	int continuer, val;
	char menu;
	struct timeval timer;
	long microsec, delta;

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

	printd("Affichage des infos du client + menu");

	refresh();
	continuer = 1;
	val = 0;
	gettimeofday(&timer, NULL);
	microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
	printMenu();
	while(continuer){
		menu = getch();
		gettimeofday(&timer, NULL);
		delta = timer.tv_sec*1000000+timer.tv_usec-microsec;
		if(delta > 1000000){
			mvprintw(10,0,"Statistiques");
			mvprintw(11,0,"Temps d'execution : %d", val);
			val++;
			microsec = (unsigned long long)timer.tv_sec*1000000+timer.tv_usec;
		}
		move(15,0);
		if(menu != ERR){
			clrtobot();
			refresh();
		}
		switch(menu){
			case '1':
				printd("Affichage des clients");
				break;
			case '2':
				printFileList(fileList);
				break;
			case '3':
				printd("Connection");
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
	endwin();

	return 0;
}

void printMenu(){
	clear();
	attron(A_BOLD | A_UNDERLINE);
	printw("==== Menu ====\n");
	printw("1 - Afficher les clients\n");
	printw("2 - Afficher les fichiers possédés\n");
	printw("3 - Se connecter à un client\n");
	printw("4 - Do stuff\n");
	printw("\n");
	printw("q - Quit\n");
	attroff(A_BOLD | A_UNDERLINE);
}