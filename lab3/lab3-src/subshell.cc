#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

extern char *path;

void subshell(char * text) {
	int fdpipe[2];

	int defaultin = dup(0);
	int defaultout = dup(1);

	int pid;

	if (pipe(fdpipe) == -1) {
		perror("bad pipe\n");
		exit(2);
	}
	
	dup2(fdpipe[0], 0);
	dup2(fdpipe[1], 1);

	pid = fork();
	if (pid == -1) {
		perror(": fork\n");
		exit(2);
	}

	if (pid == 0) {
		write(fdpipe[1], "\n", 1);
		write(fdpipe[1], text, strlen(text));
		write(fdpipe[1], "\n", 1);
		write(fdpipe[1], "exit\n", 5);
		close(fdpipe[0]);
		close(fdpipe[1]);

		//child		
		char *args[2];
		args[0] = path;
		args[1] = NULL;
		
		dup2(defaultout, 1);
		fprintf(stderr, "==~~~~== %s\n", text);
		printf("==~=~== %s\n", path);

	//close( defaultin );
	close( defaultout );

		execvp(path, args);
		perror(": exec ");
		perror(path);
		_exit(2);
	}

	fprintf(stderr, "=====2\n");

	//waitpid(pid, 0, 0);
	sleep(10);

	char a = 3;
	write(fdpipe[1], &a, 1);
	close(fdpipe[1]);

	char c;
	char subshell_out[1024];
	read(fdpipe[0], &c, 1);
	int i = 0;


	fprintf(stderr, "=====3\n");
	while (c != 3) {
		if (c == '\n') {
			subshell_out[i] = ' ';
		} else {
			subshell_out[i] = c;
		}
		i ++;
		read(fdpipe[0], &c, 1);
	}

	subshell_out[i] = 0;

	fprintf(stderr, "=====4\n");

	close(fdpipe[0]);
	close(fdpipe[1]);
	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	close( defaultin );
	close( defaultout );

	fprintf(stderr, "~~~~~~~~~~~~~%s\n", subshell_out);

}
