#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
// #define PATH_MAX 1024
void check()
{
	fflush(stdout);
	sleep(1);
}
int checkifexecutable(const char *filename)
{
     int result;
     struct stat statinfo;
     
     result = stat(filename, &statinfo);
     if (result < 0) return 0;
     if (!S_ISREG(statinfo.st_mode)) return 0;

     if (statinfo.st_uid == geteuid()) return statinfo.st_mode & S_IXUSR;
     if (statinfo.st_gid == getegid()) return statinfo.st_mode & S_IXGRP;
     return statinfo.st_mode & S_IXOTH;
}
int findpathof(char *pth, const char *exe)
{
     char *searchpath;
     char *beg, *end;
     int stop, found;
     int len;
	 printf("-1");check();
     if (strchr(exe, '/') != NULL) {
	  if (realpath(exe, pth) == NULL) return 0;
	  return  checkifexecutable(pth);
     }
	 printf("0");check();
     searchpath = getenv("PATH");
     if (searchpath == NULL) return 0;
     if (strlen(searchpath) <= 0) return 0;
	 printf("1");check();
     beg = searchpath;
	 printf("2");check();
     stop = 0; found = 0;
     do {
		 printf("3");check();
       printf(",%s",beg);check();
	  end = strchr(beg, ':');
       printf("3r");check();
	  if (end == NULL) {
	       stop = 1;
		   printf("4");check();
	       strncpy(pth, beg, PATH_MAX);
		   printf("5");check();
	       len = strlen(pth);
		   printf("6");check();
	  } else {
	       strncpy(pth, beg, end - beg);
		   printf("7");check();
	       pth[end - beg] = '\0';
		   printf("8");check();
	       len = end - beg;
		   printf("9");check();
	  }
	  if (pth[len - 1] != '/') strncat(pth, "/", 1);
	  strncat(pth, exe, PATH_MAX - len);
	  printf("10");check();
	  found = checkifexecutable(pth);
	  printf("11");check();
	  if (!stop) beg = end + 1;
	  printf("12");check();
     } while (!stop && !found);
	  
     return found;
}



int main(int argc, char **argv)
{
     char path[PATH_MAX+1];
     char *progpath = strdup(argv[0]);
     char *prog = basename(progpath);
     char *exe;

     if (argc != 2) {
	  fprintf(stderr, "Usage:  %s <executable file>\n", prog);
	  return 1;
     }
     exe = argv[1];

     if (!findpathof(path, exe)) {
	  fprintf(stderr, "No executable \"%s\" found\n", exe);
	  return 1;
     }
     puts(path);
     free(progpath);
     return 0;
}