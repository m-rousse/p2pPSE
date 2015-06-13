#ifndef DATASPEC_INCLUDE_H
#define DATASPEC_INCLUDE_H

/* module datathread : donnees specifiques */

/* donnees specifiques */
typedef struct DataSpec_t {
  pthread_t id;					/* identifiant du thread */
  int libre;					/* indicateur de terminaison */
  int tid;						/* identifiant logique */
  int canal;						/* canal de communication */
  sem_t sem;						/* semaphore de reveil */
  int socketID;					//Identifiant du socket
  int clientIP;					//Adresse IP du client connecté
  int quit;
} DataSpec;

#endif
