#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINE_LENGTH 100

const int ARGV_LENGTH = LINE_LENGTH/2+1;
//parse the line, sort arguments into argv
void parseLine (char *line, char *argv[ARGV_LENGTH]){
	char *s = line, *e = line;
	unsigned int i = 0;
	while (i < ARGV_LENGTH-1){ //for safety

		while (*e != '\0' && !isspace(*e)) e++;
		printf("ei = %d\n", e-line);
		size_t leng = e-s;
		if (leng > 0){
			argv[i] = malloc(leng+1); //leng and '\0'
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
}

int main (){ //args later
	char *args[ARGV_LENGTH];
	int run = 1;

	while (run) {
		printf("bashic $ > ");
		
		char cmdline[LINE_LENGTH];
		gets(cmdline);
		// ls -la as as as a

		char *cmd[] = { "ls", "-l", (char *)0 };

		pid_t pid = fork();
		if (pid > 0){ //parent
			//wait(&status);
			printf("exec done\n");
		} else if (pid == 0){ //child
			printf("child here\b");
			execvp(cmd[0], cmd);
		} else { //fork failed
			printf ("Fork failed\n");
			break;
		}
	}




	return 0;
}
