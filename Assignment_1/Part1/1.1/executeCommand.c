#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int executeCommand (char *cmd) {
	char* params[20];
	char* token = strtok(cmd, " ");
	int i=0;
	while (token!=NULL){
		params[i++] = token;
		token = strtok(NULL, " ");
	}
	params[i] = NULL;
	char cmd_name[20];
	strcpy(cmd_name, params[0]);

	char* envvar = "CS330_PATH";
	char* path = getenv(envvar);
	if(path==NULL){
        printf("UNABLE TO EXECUTE\n");
        return -1;
    }
	int pid = fork();
	if (pid<0){
		printf("UNABLE TO EXECUTE\n");
        return -1;
	}
	if (!pid){
		char* token = strtok(path, ":");
		while (token!=NULL){
			char path_to[128]; path_to[0] = '\0';
			strcat(path_to, token); strcat(path_to, "/"); strcat(path_to, cmd_name);
			params[0] = path_to;
			int ret = execv(params[0], params);
			token = strtok(NULL, ":");
		}
		printf("UNABLE TO EXECUTE\n");
		exit(-1);
	}
	int stat;
	wait(&stat);
	return WEXITSTATUS(stat);
}

int main (int argc, char *argv[]) {
	if (argc<2){
		printf("UNABLE TO EXECUTE\n");
		return -1;
	}
	return executeCommand(argv[1]);
}