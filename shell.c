#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include <linux/limits.h>
#include <libgen.h>
#include<sys/types.h> 
#include<sys/stat.h> 
#include<sys/wait.h>
#include<readline/readline.h> 
#include<fcntl.h>
#include <limits.h>
//to make command history 
//install this library by followng command //////////////////////sudo apt-get install libreadline-dev/////////////////////
#include<readline/history.h> 

#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 

// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J") 

/*
* //TODO
* - Commands like "cd", "cd /", "cd ~" are not observed.
*/
char userr[1000];
int no_of_pipes = 0;
// will be executed when shell will start

char* remove_spaces(char* s)
{
	char *temp;
	for(int i=0;;i++)
	{
		if(s[i]=='\0' || s[i]!=' ')
		return temp = s+i;
	}
}
void init_shell() 
{ 
	clear(); 
	printf("\nOperating Systems Mini Project");
	printf("\n*******************************"); 
	char* username = getenv("USER"); 
	printf("\n\n\nDefault user is: @%s", username); 
	printf("\nEnter your username for terminal\n");
	char* temp;
	temp = readline("");
	temp = remove_spaces(temp);
	if(!strlen(temp))temp = username;
	strcpy(userr,temp);
	clear();
} 

// Function to take input 
int takeInput(char* str) 
{ 
	char* buf;
	buf = readline("\n>>> ");
	buf = remove_spaces(buf); 
	if (strlen(buf) != 0) { 
		// Add to history if buffer length is more than zero
		add_history(buf);
		strcpy(str, buf); 
		return 0; 
	} else { 
		return 1; 
	} 
} 

void printDir() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	char hostname[HOST_NAME_MAX];
	gethostname(hostname,HOST_NAME_MAX);
	printf("\n%s@%s: %s",userr,hostname,cwd); 
} 

void execArgs(char*** parsed) 
{
    char *file = parsed[0][0],*inp=NULL,*out=NULL;
    char *argv[MAXLIST];
    for(int i=0;parsed[0][i]!=NULL;i++)
    {
        argv[i]=parsed[0][i];
		if(i>0 && argv[i-1]!=NULL && strcmp(argv[i-1],"<")==0)
		{
			inp = argv[i];
			argv[i-1]=argv[i]=NULL;
		}
		if(i>0 && argv[i-1]!=NULL && strcmp(argv[i-1],">")==0)
		{
			out = argv[i];
			argv[i-1]=argv[i]=NULL;
		}
    }
	pid_t pid = fork(); 
	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	} else if (pid == 0) { 
		if(inp!=NULL)
		{
			int fp = open(inp,O_RDONLY);
			dup2(fp,STDIN_FILENO);
		}
		if(out!=NULL)
		{	
			int fp = open(out,O_CREAT | O_APPEND | O_WRONLY);
			dup2(fp,STDOUT_FILENO);
		}
		if (execvp(file, argv) < 0) { 
			printf("\nCould not execute command.."); 
		}
		exit(0); 
	} else { 
		wait(NULL); 
		return; 
	} 
} 

void execArgsPiped(char*** parsed) 
{ 
	int fd[2],commandno = 0,fdd=0;
	pid_t pid;
	while(commandno<=no_of_pipes)
	{
		pipe(fd);
		pid = fork();
		if(pid<0)
		{
			printf("fork failed when %d\n",commandno);
		}
		if(pid==0)
		{
			int i;
			char *file = parsed[commandno][0],*inp=NULL,*out=NULL;
  			char *argv[MAXLIST];
    		for(i=0;parsed[commandno][i]!=NULL;i++)
    		{
    		    argv[i]=parsed[commandno][i];
				if(i>0 && argv[i-1]!=NULL && strcmp(argv[i-1],"<")==0)
				{
					inp = argv[i];
					argv[i-1]=argv[i]=NULL;
				}
				if(i>0 && argv[i-1]!=NULL && strcmp(argv[i-1],">")==0)
				{
					out = argv[i];
					argv[i-1]=argv[i]=NULL;
				}
    		}
			for(;i<MAXLIST;i++)
			{
				argv[i] = NULL;
			}
			if(inp!=NULL)
			{
				int fp = open(inp,O_RDONLY);
				dup2(fp,STDIN_FILENO);
			}
			else
			{
				dup2(fdd,0);	
			}
			if(out!=NULL)
			{	
				int fp = open(out,O_CREAT | O_APPEND | O_WRONLY);
				dup2(fp,STDOUT_FILENO);
			}
			else if(commandno!=no_of_pipes && dup2(fd[1],1)<0)
			{
				printf("dup2 failed when %d\n",commandno);
			}
			close(fd[0]);
			if(execvp(file,argv)<0)
			{
				printf("excvp failed at %d",commandno);
			}
			exit(0);
		}
		else
		{
			close(fd[1]);
			wait(NULL);
			fdd = fd[0];
			commandno++;
		}
	}	
}

void openHelp() 
{ 
	puts("\n***Operating System Mini Project***"
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		"\n>pipe handling"
		"\n>improper space handling"); 
     	return; 
} 
void check()
{
	fflush(stdout);
	sleep(2);
}
int checkifexecutable(const char *filename)
{
	printf("%s\n",filename);
     int result;
     struct stat statinfo;
     
     result = stat(filename, &statinfo);
     if (result < 0) return 0;
     if (!S_ISREG(statinfo.st_mode)) return 0;

     if (statinfo.st_uid == geteuid()) return statinfo.st_mode & S_IXUSR;
     if (statinfo.st_gid == getegid()) return statinfo.st_mode & S_IXGRP;
     return statinfo.st_mode & S_IXOTH;
}
int findpathof(char *pth,char *exe)
{
     char *searchpath;
     char *beg, *end;
     int stop, found;
     int len;
     searchpath = getenv("PATH");
     if (searchpath == NULL) return 0;
     if (strlen(searchpath) <= 0) return 0;
     beg = searchpath;
     stop = 0; found = 0;
	 printf("trying in these paths\n");
     do {
	  end = strchr(beg, ':');
	  if (end == NULL) {
	       stop = 1;
	       strncpy(pth, beg, PATH_MAX);
	       len = strlen(pth);
	  } else {
		   long int temp = end-beg;
	       strncpy(pth, beg, temp);
	       pth[end - beg] = '\0';
	       len = end - beg;
	  }
	  if (pth[len - 1] != '/') strcat(pth, "/");
	  strncat(pth, exe, PATH_MAX - len);
	  found = checkifexecutable(pth);
	  if (!stop) beg = end + 1;
     } while (!stop && !found);
	  
     return found;
}


int ownCmdHandler(char** parsed) 
{ 
	//Custom commands specific to our shell
	int NoOfOwnCmds = 4, i, switchOwnArg = 0; 
	char* ListOfOwnCmds[NoOfOwnCmds]; 
	char* username,path[5000]; 

	ListOfOwnCmds[0] = "exit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "help"; 
	ListOfOwnCmds[3] = "wch"; 
	for (i = 0; i < NoOfOwnCmds; i++) { 
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
			switchOwnArg = i + 1; 
			break; 
		} 
	} 

	switch (switchOwnArg) { 
	case 1: 
		printf("\nGoodbye\n"); 
		exit(0); 
	case 2: 
		chdir(parsed[1]); 
		return 1; 
	case 3: 
		openHelp(); 
		return 1; 
	case 4:
		if (!findpathof(path, parsed[1])) {
	  	printf("No executable \"%s\" found\n", parsed[1]);
     	}
		else
		{
			printf("found it here : ");
		 	puts(path);
		}
		return 1;
	default: 
		break; 
	} 

	return 0; 
} 

void parsePipe(char* str, char** strpiped) 
{ 
	int i; 
	for (i = 0; i < MAXLIST; i++) { 
		strpiped[i] = strsep(&str, "|"); 
		if (strpiped[i] == NULL) 
		{no_of_pipes = i-1; break;}
	}
} 

void parseSpace(char* str, char** parsed) 
{ 
	int i; 
    str = remove_spaces(str);
	for (i = 0; i < MAXLIST; i++) {
		char *temp;
		parsed[i] = strsep(&str, " ");
		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	}
} 

int processString(char* str, char*** parsed) 
{ 

	char* strpiped[MAXLIST]; 
	parsePipe(str, strpiped); 
    for(int i=0;i<=no_of_pipes;i++)
		parseSpace(strpiped[i], parsed[i]);
    if(no_of_pipes==0 && ownCmdHandler(parsed[0]))
        return 0;
	return 1+(no_of_pipes>0) ; 
} 
int main() 
{ 
	char inputString[MAXCOM], ***parsedArgs; 
	parsedArgs = calloc(MAXLIST,sizeof(char**));
	for(int z = 0; z < MAXLIST; z++) { 
    	parsedArgs[z] = calloc(MAXLIST, sizeof(char*));
    	for(int i = 0; i < MAXLIST; i++) {
        	parsedArgs[z][i] = calloc(MAXLIST, sizeof(char));
    	}
	}
	int execFlag = 0; 
	init_shell(); 
	while (1) { 
		printDir();
		// Make sure input len > 0. Also save in buffer if > 0.
		if (takeInput(inputString)) 
			continue;  
		execFlag = processString(inputString, parsedArgs); 
		if (execFlag == 1) 
			execArgs(parsedArgs);
		if (execFlag == 2) 
			execArgsPiped(parsedArgs); 
	} 
	return 0; 
} 

