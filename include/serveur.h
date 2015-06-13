#include "pse.h"
#include "fichiers.h"

#define NBWORKERS 5
#define ATTENTE 2000
#define PORT 24240

void *traitement(void *arg);
void *traitementRecherche(void *arg);
void *traitementAnnonce(void *arg);
void *affichageFichiers(void *arg);
void *finConnexion(void *arg);
