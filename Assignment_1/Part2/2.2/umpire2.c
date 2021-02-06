#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "gameUtils.h"

int getWalkOver(int numPlayers); // Returns a number between [1, numPlayers]

char* read_line(int fd){
	char* line = (char*)malloc(200*sizeof(char));
	int i = 0;
	char ch;
	while (read(fd, &ch, 1)){
		if (ch!='\n') line[i++] = ch;
		else break;
	}
	line[i] = '\0';
	if (i) return line;
	return NULL;
}

int get_active_print(char* players[], int num){
	int ret = 0;
	for (int i=0; i<num; i++){
		if (players[i]!=NULL){
			if (ret) printf(" ");
			ret++;
			printf("p%d", i);
		}
	}
	if (ret!=1) printf("\n");
	return ret;
}

int result(char ch1, char ch2){
	if (ch1==ch2) return 0;
	if (ch1=='0'){
		if (ch2=='1') return 2;
		else return 1;
	}
	else if (ch1=='1'){
		if (ch2=='0') return 1;
		else return 2;
	}
	else {
		if (ch2=='0') return 2;
		else return 1;
	}
}

int len(char* str){
	if (str==NULL) return 0;
	int ret=0;
	while (str[ret]!='\0') ret++;
	return ret;
}

int check(char* str){
	if (len(str)!=2) return 0;
	if (str[0]=='-' && str[1]=='r') return 1;
	return 0;
}

int main(int argc, char *argv[]){
	int n;
	if (argc==4){
		if (!check(argv[1])) return -1;
		n = atoi(argv[2]);
	} 
	else if (argc==2) n = 10;
	else return -1;

	int file = open(argv[argc-1], O_RDONLY);
	if (file<0) return -1;
	int ids = atoi(read_line(file));
	char* players[ids];
	for (int i=0; i<ids; i++){
		players[i] = read_line(file);
	}

	int fd1[ids][2];
	int fd2[ids][2];
	for (int i=0; i<ids; i++){
		if (pipe(fd1[i])<0) return -1;
		if (pipe(fd2[i])<0) return -1;
		int pid = fork();
		if (pid<0) return -1;
		if (!pid){
			close(fd1[i][1]); close(fd2[i][0]);
			dup2(fd1[i][0], STDIN_FILENO); dup2(fd2[i][1], STDOUT_FILENO);
			int ret = execl(players[i], players[i], NULL);
			exit(-1);
		}
		close(fd2[i][1]);
	}
	int act_num = get_active_print(players, ids);
	while (act_num!=1){
		int walk = -1;
		if (act_num%2) walk = getWalkOver(act_num);
		
		int matches = act_num/2;
		int rank = 0;
		int j = 0;
		int pairs[2*matches];
		for (int i=0; i<ids; i++){
			if (players[i]!=NULL){
				rank++;
				if (walk==rank) continue;
				else {
					pairs[j++] = i;
				}
			}
		}
		int rounds = n;
		int scores[2*matches];
		for (int i=0; i<2*matches; i++) scores[i] = 0;
		while (rounds--){
			for (int i=0; i<2*matches; i+=2){
				// match between ids pairs[i] and pairs[i+1]
				int id1 = pairs[i]; int id2 = pairs[i+1];
				if (write(fd1[id1][1], "GO", 2)<=0) return -1;
				char p1;
				if (read(fd2[id1][0], &p1, 1)<=0) return -1;
				if (p1!='0' && p1!='1' && p1!='2') return -1;
				if (write(fd1[id2][1], "GO", 2)<=0) return -1;
				char p2;
				if (read(fd2[id2][0], &p2, 1)<=0) return -1;
				if (p2!='0' && p2!='1' && p2!='2') return -1;

				int res = result(p1, p2);
				if (res==1) scores[i]++;
				else if (res==2) scores[i+1]++;
			}
		}
		for (int i=0; i<2*matches; i+=2){
			int id = scores[i]>=scores[i+1] ? pairs[i+1] : pairs[i];
			players[id] = NULL;
			close(fd1[id][1]); close(fd2[id][0]);
		}
		act_num = get_active_print(players, ids);
	}
	for (int i=0; i<ids; i++){
		if (players[i]!=NULL){
			close(fd1[i][1]); close(fd2[i][0]); close(fd1[i][0]);
		}
	}
	return 0;
}