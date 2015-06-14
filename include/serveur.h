#include "pse.h"
#include <filelist.h>

#define NBWORKERS 5
#define ATTENTE 2000
#define PORT 24240

void *traitement(void *arg);
void traitementRecherche(DataSpec *data);
void traitementAnnonce(DataSpec *data);
void affichageFichiers(DataSpec *data);
void quitServer(DataSpec *data);
void disconnectClient(DataSpec *data);
void sendFileClient(DataSpec *data);