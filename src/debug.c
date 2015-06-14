#include <debug.h>

void printd(char* message, ...){
  char *tmp, *buf;
  char format[] = "[%s : %s] ";
  va_list vl;
  va_start(vl,message);

  buf = malloc(LIGNE_MAX*sizeof(char));
  snprintf(buf, LIGNE_MAX, "%d", getpid());
  int size = strlen(getenv("_"))+strlen(format)+strlen(buf)-2-2+1;
  tmp = (char*) malloc(sizeof(char)*size);
  snprintf(tmp, size, format, getenv("_"),buf);
  free(buf);
  size = strlen(tmp)+2;
  tmp = realloc(tmp,sizeof(char)*size);
  strcat(tmp,"%s");
  buf = malloc(sizeof(char)*LIGNE_MAX);
  snprintf(buf,LIGNE_MAX,tmp, message);
  //vfprintf(stderr, buf, vl);
  if(stdscr != 0)
    vwprintw(stdscr,buf, vl);
  else
    printf("%s",buf);
  free(tmp);
  free(buf);
  va_end(vl);
}
