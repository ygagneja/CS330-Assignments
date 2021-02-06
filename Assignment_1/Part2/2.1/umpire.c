#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

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

int main(int argc, char* argv[]) {
	int s1 = 0;
	int s2 = 0;

	if (argc!=3) return -1;

	// player 1
	int p1fd1[2]; if (pipe(p1fd1)<0) return -1;
	int p1fd2[2]; if (pipe(p1fd2)<0) return -1;
	
	int p1 = fork();
	if (p1<0) return -1;
	if (!p1){
		// child 1
		close(p1fd1[1]); close(p1fd2[0]);
		dup2(p1fd1[0], STDIN_FILENO); dup2(p1fd2[1], STDOUT_FILENO);
		int ret = execl(argv[1], argv[1], NULL);
		exit(-1);
	}
	close(p1fd2[1]);

	// player 2
	int p2fd1[2]; if (pipe(p2fd1)<0) return -1;
	int p2fd2[2]; if (pipe(p2fd2)<0) return -1;

	int p2 = fork();
	if (p2<0) return -1;
	if (!p2){
		// child 2
		close(p2fd1[1]); close(p2fd2[0]);
		dup2(p2fd1[0], STDIN_FILENO); dup2(p2fd2[1], STDOUT_FILENO);
		int ret = execl(argv[2], argv[2], NULL);
		exit(-1);
	}
	close(p2fd2[1]);

	int mat = 10;
	while (mat--){
		// send signal to p1
		if (write(p1fd1[1], "GO", 2)<=0) return -1;
		char p1;
		if (read(p1fd2[0], &p1, 1)<=0) return -1;
		if (p1!='0' && p1!='1' && p1!='2') return -1;
		// send signal to p2
		if (write(p2fd1[1], "GO", 2)<=0) return -1;
		char p2;
		if (read(p2fd2[0], &p2, 1)<=0) return -1;
		if (p2!='0' && p2!='1' && p2!='2') return -1;
		int res = result(p1, p2);
		if (res==1) s1++;
		else if (res==2) s2++;
	}

	close(p1fd1[0]); close(p2fd1[0]); close(p1fd1[1]); close(p1fd2[0]); close(p2fd1[1]); close(p2fd2[0]);

	printf("%d %d", s1, s2);
	return 0;
}