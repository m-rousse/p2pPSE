#include <debug.h>

void printd(char* message){
  fprintf(stderr, "[%s] %s\n", getenv("_"), message);
}
