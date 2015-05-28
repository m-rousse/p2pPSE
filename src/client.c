#include <client.h>

int main(){
	sFileList* fileList;

	printd("Démarrage");

	// Initialisation de la liste des fichiers possédés
	fileList = initFileList();
	restoreFileList(fileList);

	printd("Annonce des fichiers possédés");
	sendFileList();

	printd("Lancement du thread d'UL");
	printd("Lancement du thread de DL");
	
	printd("Sauvegarde de la liste des fichiers possédés");
	saveFileList(fileList);
	return 0;
}