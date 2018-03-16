#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include  <signal.h>
#include <sys/types.h>

//Colors:
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define LINE_LENGTH 100
#define HISTORY_LENGTH 100
#define RHISTORY_LENGTH 10 //recent history for the "history" cmd

#define ARGV_LENGTH (LINE_LENGTH/2+1)

int run;

int hi, hcnt;
char history[HISTORY_LENGTH][LINE_LENGTH+5];

pid_t pid;

void displayPrompt();

//parse the line, sort arguments into argv
void parseLine (char *line, char *argv[], int *argcntp);
//frees the memory used to split and store the parameters
void clearArgv(char *argv []);

//return the status of the fork, reports execution errors
int forkExec(char *args[], int *argcnt);

//kills all children who did not yet terminate
void interruptChildren ();
void interruptHandler(int sig);

//store cmd in history
void historize(char *src, char history[][LINE_LENGTH+5]);
void printHistory();
int historyNum(char *cmd);

//utilities
int max(int a, int b);
int min(int a, int b);

int main (){ //args later
	signal(SIGINT, interruptHandler);

	char *args[ARGV_LENGTH];
	int argcnt;

	char cmdline[LINE_LENGTH+5];

	int execHistory, hprev;

	run = 1;
	execHistory = 0;
	hi = hcnt = 0;
	do {

		displayPrompt();

		argcnt = 0;
		char *src;
		if (!execHistory){
			fgets(cmdline, LINE_LENGTH, stdin);
			src = cmdline;
		} else{
			src = history[hprev];
			printf("%s", history[hprev]);
			execHistory = 0;
		}
		parseLine(src, args, &argcnt);

		waitpid(-1, NULL, WNOHANG); 		//to detect terminations in bg

		if (argcnt < 1)
			continue;

		//commands not stored in history:
		int hnum;
		hprev = -1;
		if (strcmp(args[0], "exit") == 0)
			run = 0;
		else if (strcmp(args[0], "history") == 0)
			printHistory();
		else
		   	if ((hnum = historyNum(args[0])) != -1){ // !! / !N
			//1-based
			if (hnum < max(hcnt-HISTORY_LENGTH+1, 1) || hnum > hcnt)
				printf("Error executing a history command (invalid number)\n");
			else{
				hnum--; //0-based
				hprev = hnum%HISTORY_LENGTH;
				execHistory = 1;
			}
		}
		//commands stored in history:
		else {
			historize(src, history);

			if (strcmp(args[0], "cd") == 0){
				if (chdir(args[1]))
					printf("Error: Invalid path for cd\n");
			} else {
				if (forkExec(args, &argcnt) == 0)
					printf("Fork failed...\n");
			}
		}

	} while (clearArgv(args), run);

	interruptChildren();
	return 0;
}

void displayPrompt(){
	//later: colorize, username, etc
	char cwd[LINE_LENGTH];
	getcwd(cwd, LINE_LENGTH);

	printf("|" KBLU "bashic " KCYN "$ " KNRM"[" KYEL "%s" KNRM "]\n", cwd);
	printf(KNRM "|=> ", KNRM); 
	fflush(stdout);
}

void parseLine (char *line, char *argv[], int *argcntp){
   	char *s = line, *e = line;
   	int i = 0;
	while (i < ARGV_LENGTH-1){ //for safety

		while (*e != '\0' && !isspace(*e)) e++;
		size_t leng = e-s;
		if (leng > 0){
			argv[i] = malloc(leng+1); //leng and '\0'
			if (argv[i] == (char *)0){
				printf ("Out of memory - Terminating...\n");
				exit(-1);
			}
			strncpy(argv[i], s, leng);
			strncpy(argv[i]+leng, "\0", 1);
			i++;
		}

		if (*e == '\0')
			break;
		else
			s = e = e+1;
	}
	argv[i] = (char *)0;	 //null pointer
	*argcntp = i;
}

//frees the memory used to split and store the parameters
void clearArgv(char *argv []){
	char **p = argv, **q = argv;
	int i;
	for (i = 0; i < ARGV_LENGTH-1 && argv[i] != (char *)0; i++){
		//printf("%d\n", argv[i]);
		free(argv[i]);
	}
}

int forkExec(char *args[], int *argcntp){
	int argcnt = *argcntp;
	int background = 0, child_status;

	if (strcmp(args[argcnt-1], "&") == 0){
		free(args[argcnt-1]);
		args[argcnt-1] = (char*)0;
		background = 1;
		argcnt--;
	}

	pid = fork();
	int ret = 1;
	if (pid > 0){ //parent
		if (!background)
			waitpid(pid, &child_status, 0);
	} else if (pid == 0){ //child
		if (execvp(args[0], args) == -1){
			printf("Error executing...\n");
			kill(getpid(), SIGKILL);
		}
	} else{ 
		ret = 0;
		printf("Error forking...\n");
	}

	*argcntp = argcnt;
	return ret;
}

//kills all children who did not yet terminate
void interruptChildren (){
	printf("Interrupting all children...\n");
	signal(SIGINT, SIG_IGN);
	kill(-pid, SIGINT);
	signal(SIGINT, interruptHandler);
}

void interruptHandler(int sig){
	printf ("\nBashic interrupted!\nUse \"exit\" if you want to quit..\n");
}

int max(int a, int b){
	return a>b? a : b;
}

int min(int a, int b){
	return a<b? a : b;
}

void historize(char *src, char history[][LINE_LENGTH+5]){
	strncpy(history[hi], src, strlen(src)+1);
	hi = (hi+1)%HISTORY_LENGTH;
	hcnt++;
}

void printHistory(){
	int i = hi-1, c = hcnt, j;
	int rlength = min(RHISTORY_LENGTH, hcnt);
	for (j = 0; j < rlength; j++){
		if (i == -1)
			i = HISTORY_LENGTH-1;
		printf("%4d: %s", c, history[i]);
		i--, c--;
	}
}

int historyNum(char *cmd){
	int num = -1;
	if (strcmp(cmd, "!!") == 0)
		num = hcnt;
	else if (cmd[0] == '!'){
		int n = strlen(cmd);
		char *numc = malloc(n);
		strncpy(numc, cmd+1, n); //to include '\0'
		num = 1;

		int i;
		for (i = 0; i < n-1 && num; i++)
			if (!isdigit(numc[i])) 
				num = -1;
		if (num)
			num = atoi(numc);

		free(numc);
	}

	return num;
	//0: not a history cmd
	//value of num
}
