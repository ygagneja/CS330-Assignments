#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

char* read_command(int fd){
	char ch;
	char* str = (char*)malloc(sizeof(char)*64);
	int i=0;
	while (read(fd, &ch, 1)){
		if (ch!='\n') str[i++] = ch;
		else break;
	}
	str[i] = '\0';
	if (i) return str;
	return NULL;
}

int execute_in_parallel(char *infile, char *outfile){
	int fd1 = open(infile, O_RDONLY); // open infile
	if (fd1<0) return -1;
	// fill path with variable value
	char* envvar = "CS330_PATH";
	char* path = getenv(envvar);
	if (path==NULL) return -1;
	char* cmds[50];
	char* cmd = read_command(fd1);
	int len = 0;
	while (cmd!=NULL){
		cmds[len++] = cmd;
		cmd = read_command(fd1);
	}
	int fd[len][2];
	for (int i=0; i<len; i++){
		if (pipe(fd[i])<0) return -1;
		int pid = fork();
		if (pid<0) return -1;
		if (!pid){
			//child
			close(fd[i][0]);
			dup2(fd[i][1], STDOUT_FILENO);
			char* params[20];
			char* token = strtok(cmds[i], " ");
			int j=0;
			while (token!=NULL){
				params[j++] = token;
				token = strtok(NULL, " ");
			}
			params[j] = NULL;
			char cmd_name[20];
			strcpy(cmd_name, params[0]);

			token = strtok(path, ":");
			while (token!=NULL){
				char path_to[128]; path_to[0] = '\0';
				strcat(path_to, token); strcat(path_to, "/"); strcat(path_to, cmd_name);
				params[0] = path_to;
				int ret = execv(params[0], params);
				token = strtok(NULL, ":");
			}
			printf("UNABLE TO EXECUTE\n");
			return -1;

		}
		close(fd[i][1]);
	}
	

	for (int i=0; i<len; i++) wait(NULL);
	// flush the pipe data to outfile
	int fd2 = open(outfile, O_CREAT|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO); // open outfile
	if (fd2<0) return -1;
	dup2(fd2, STDOUT_FILENO);
	for (int i=0; i<len; i++){
		// read from fd[i][0]
		char ch;
		while (read(fd[i][0], &ch, 1)){
			printf("%c", ch);
		}
	}

	return 0;
}

int main(int argc, char *argv[]){
	return execute_in_parallel(argv[1], argv[2]);
}