#include <debug.h>

void printd(char* message, ...){
  char *tmp, *buf;
  char format[] = "[%s] ";
  va_list vl;
  va_start(vl,message);

  int size = strlen(getenv("_"))+strlen(format)-1;
  tmp = (char*) malloc(sizeof(char)*size);
  snprintf(tmp, size, format, getenv("_"));
  size = strlen(tmp)+2;
  tmp = realloc(tmp,sizeof(char)*size);
  strcat(tmp,"%s");
  buf = malloc(sizeof(char)*LIGNE_MAX);
  snprintf(buf,LIGNE_MAX,tmp, message);
  //vfprintf(stderr, buf, vl);
  vwprintw(stdscr,buf, vl);
  free(tmp);
  free(buf);
  va_end(vl);
}
