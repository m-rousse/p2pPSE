#include "pse.h"
#include "fichiers.h"

#define NBWORKERS 5
#define ATTENTE 2000
#define PORT 24240

void *traitement(void *arg);
void traitementRecherche(DataSpec *data);
void traitementAnnonce(DataSpec *data);
void affichageFichiers(DataSpec *data);
void finConnexion(DataSpec *data);
