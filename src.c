#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include  <signal.h>
#include <sys/types.h>

#define LINE_LENGTH 100
#define HISTORY_LENGTH 100
#define RHISTORY_LENGTH 10 //recent history for the "history" cmd

const int ARGV_LENGTH = LINE_LENGTH/2+1;
int run;

//parse the line, sort arguments into argv
void parseLine (char *line, char *argv[ARGV_LENGTH], int *cntp){
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
	*cntp = i;
}

//frees the memory used to split and store the parameters
void clearArgv(char *argv [ARGV_LENGTH]){
	char **p = argv, **q = argv;
	for (int i = 0; i < ARGV_LENGTH-1 && argv[i] != (char *)0; i++){
		//printf("%d\n", argv[i]);
		free(argv[i]);
	}
}

//kills all children who did not yet terminate 
void killChildren (){
	printf("Interrupting all children :(\n");
	kill(0, SIGINT);
}

void interruptHandler(int sig){
	run = 0;
	printf("Exiting bashic:(\nPress anything to exit...\n");
}
int max(int a, int b){
	return a>b? a : b;
}
int main (){ //args later
	signal(SIGINT, interruptHandler);
	char *args[ARGV_LENGTH];
	char cmdline[LINE_LENGTH+5];
	char cwd[LINE_LENGTH];
	char history[HISTORY_LENGTH][LINE_LENGTH+5];
	int child_status;
	int background;
	int cnt;
	int hi, hcnt, hprev;
	pid_t pid;

	run = 1;
	hi = hcnt = 0;
	do {
		background = 0;
		hprev = -1;

		getcwd(cwd, LINE_LENGTH);
		printf("bashic $ (%s)> ", cwd);
		fgets(cmdline, LINE_LENGTH, stdin);
		parseLine(cmdline, args, &cnt);
		waitpid(-1, NULL, WNOHANG); 		//to detect terminations in bg

		if (cnt < 1)
			continue;
		
		if (!strcmp(args[0], "exit")){
			run = 0;
			printf ("Exiting shell..\n");
			continue;
		} else if (!strcmp(args[0], "history")){
			int i = hi-1, c = hcnt, j;
			for (j = 0; j < RHISTORY_LENGTH && j < hcnt; j++){
				if (i == -1)
					i = HISTORY_LENGTH-1;
				printf("%4d: %s", c, history[i]);
				i--, c--;
			}
			continue;
		} else if (!strcmp(args[0], "!!")){
			if (hcnt == 0){
				printf("Command history is empty...\n");
				continue;
			}
			hprev = (hi == 0? HISTORY_LENGTH-1 : hi-1);
			clearArgv(args);
			parseLine(history[hprev], args, &cnt);
		} else if (args[0][0] == '!'){
			int all_dig = 0;
			int n = strlen(args[0]);
			char *numc = malloc(n);
			strncpy(numc, args[0]+1, n); //to include '\0'
			for (int i = 0; i < n-1; i++)
				if (!isdigit(numc[i])){
					printf("Error executing a history command (invalid symbol)\n");
					all_dig = 1;
					break;
				}
			if (all_dig) continue;
			int num = atoi(numc); //1-based
			if (num < max(hcnt-HISTORY_LENGTH+1, 1) || num > hcnt){
				printf("Error executing a history command (Command not in history or invalid number)\n");
				continue;
			}
			num--; //now 0-based
			hprev = num%HISTORY_LENGTH;
			free(numc);
			clearArgv(args);
			parseLine(history[hprev], args, &cnt);
		} else if (!strcmp(args[0], "cd")){
			if (chdir(args[1]))
				printf("Error: Invalid path for cd\n");
			continue;
		}

		if (hprev != -1){
			printf("> %s", history[hprev]);
		}
		char *src = (hprev == -1? cmdline : history[hprev]);
		//store in history
		strncpy(history[hi], src, strlen(src)+1);
		hi = (hi+1)%HISTORY_LENGTH;
		hcnt++;

		if (!strcmp(args[cnt-1], "&")){
			free(args[cnt-1]);
			args[cnt-1] = (char*)0;
			printf("will run %s in  background\n", args[0]);
			background = 1;
			cnt--;
		}

		//printf("# of args = %d\n", cnt);

		pid = fork();
		if (pid > 0){ //parent
			if (!background){
				waitpid(pid, &child_status, 0);
				/*
				if (WIFEXITED(child_status))
					printf("(%s) terminated normally\n", args[0]);
				*/
			}
		} else if (pid == 0){ //child
			if (execvp(args[0], args) == -1){
				printf("Error executing..\n");
				kill(getpid(), SIGKILL);
			}
		} else
			printf ("Fork failed\n");

	} while (clearArgv(args), run);

	killChildren();
	return 0;
}
