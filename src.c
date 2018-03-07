#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINE_LENGTH 100

int main (){ //args later
	int run = 1;
	int status;

	while (run){
		printf("bashic $ > ");
		
		char cmdline[LINE_LENGTH];
		gets(cmd);
		// ls -la as as as a

		char *cmd[] = { "ls", "-l", (char *)0 };

		pid_t pid = fork();
		if (pid == 0){ //child
			printf("child here\b");
			execvp(cmd[0], cmd);
		}
		else{
			wait(&status);
			printf("exec done\n");
		}


	}




	return 0;
}
