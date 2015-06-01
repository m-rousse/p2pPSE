#include <debug.h>

void printd(char* message){
  char* tmp;
  int size = strlen(getenv("_"))+strlen(message)+4;
  tmp = (char*) malloc(sizeof(char)*size);
  snprintf(tmp, size, "[%s] %s", getenv("_"), message);
  //fprintf(stderr, "%s\n\r", tmp);
  printw("%s", tmp);
  free(tmp);
}
