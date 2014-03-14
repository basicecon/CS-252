
/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "command.h"
#include "wildcarding.h"
#include "pwd.h"

char *path;

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	// implement environment variable expansion
	char *reg = ".*\\$\\{[^\\}]+\\}.*";

	if (isMatch(reg, argument)) {
		//printf("===============%s\n", argument);
		char new_arg[1024];
		int i = 0;

		while (*argument) {
			if (*argument == '$' && *(argument+1) == '{') {
				char env[102];
				int j = 0;
				argument += 2;
				while (*(argument) != '}') {
					env[j] = *argument;
					j ++;
					argument ++;
				}
				env[j] = 0;
				char *tmp = getenv(env);
				while (*tmp) {
					new_arg[i] = *tmp;
					tmp ++;
					i ++;
				}				
			} else {
				new_arg[i] = *argument;
				i ++;
			}

			argument ++;
		}
		argument = new_arg;
	}

	// implement tilde
	if (argument[0] == '~') {
		char *name;
		char new_argument[1024];
		struct passwd *pwd;
		if (strlen(argument) == 1) {
			name = getenv("HOME");
			int i;
			int j;
			for (i = 0, j = 0; i < strlen(name); i ++, j ++) {
				new_argument[j] = name[i];
			}
			new_argument[j+1] = 0;
			//printf("%s\n", new_argument);

		} else {
			int i = 1;
			int j = 0;
			name = (char *)malloc(1024);
			while (i < strlen(argument) && argument[i] != '/') {
				name[j] = argument[i];
				i ++;
				j ++;
			}

			int k = i;
			name[j] = 0;
			pwd = getpwnam(name);
			if (pwd) {
				name = strdup(pwd->pw_dir);
			} else {
				perror("bad tilde");
				return;
			}

			for (i = 0, j = 0; i < strlen(name); i ++, j ++) {
				new_argument[j] = name[i];
			}
			new_argument[j] = 0;

			if (k < strlen(argument)) {
				strcat(new_argument, argument+k);
			}
		}
		argument = new_argument;
		//printf("%s\n", argument);
	}
	_arguments[ _numberOfArguments ] = strdup(argument);

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command::clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
		printf("\n");
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	

	if (strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0) {
		//exit(2);
		_exit(0);
	}

	int fdpipe[2];

	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);

    // redirect err
	if (_errFile) {
		if (_isErrAppend == 0) {
			int errfd = creat(_errFile, 0666);
			if (errfd < 0) {
				perror("create errfile");
				exit(2);
			}
			dup2(errfd, 2);
			close(errfd);
		} else {
			int errfd = open(_errFile, O_WRONLY | O_APPEND, 0666);
			if (errfd < 0) {
				perror("open errfile");
				exit(2);
			}
			dup2(errfd, 2);
			close(errfd);
		}
	}

	int pid;
	for (int i = 0; i < _numberOfSimpleCommands; i ++) {

		if (i == 0) {
			if (_inputFile) {
				int infd = open(_inputFile, O_RDONLY, 0666);
				if (infd < 0) {
					perror(_simpleCommands[i]->_arguments[0]);
					perror(": create infile");
					exit(2);
				}
				dup2(infd, 0);
				close(infd);
			}
		} else {
			// pipe input
			dup2(fdpipe[0], 0);
			close(fdpipe[0]);
		}

		if (i == _numberOfSimpleCommands-1) {
			if (_outFile) {
				if (_isOutAppend == 0) {
					int outfd = creat(_outFile, 0666);
					if (outfd < 0) {
						perror(_simpleCommands[i]->_arguments[0]);
						perror(": create outfile");
						exit(2);
					}
					dup2(outfd, 1);
					close(outfd);
				} else {
					int outfd = open(_outFile, O_WRONLY | O_APPEND, 0666);
					if (outfd < 0) {
						perror(_simpleCommands[i]->_arguments[0]);
						perror(": open outfile");
						exit(2);
					}
					dup2(outfd, 1);
					close(outfd);
				}
			} else {
				dup2(defaultout, 1);
				close(defaultout);
			}
		} else {
			// pipe output
			if (pipe(fdpipe) == -1) {
				perror("bad pipe\n");
				exit(2);
			}
			dup2(fdpipe[1], 1);
			close(fdpipe[1]);
		}
		
		// implement setenv
		if (strcmp(_simpleCommands[i]->_arguments[0], "setenv") == 0) {
			if (_simpleCommands[i]->_arguments[1] && _simpleCommands[i]->_arguments[2]) {
				if (setenv(_simpleCommands[i]->_arguments[1], 
					_simpleCommands[i]->_arguments[2], 1) < 0) {
					perror("set environment failed");
				}
			} else {
				perror("not enough arguments");
			}
			continue;
		}

		// implement unsetenv
		if (strcmp(_simpleCommands[i]->_arguments[0], "unsetenv") == 0) {
			if (_simpleCommands[i]->_arguments[1]) {
				if (unsetenv(_simpleCommands[i]->_arguments[1]) < 0) {
					perror("unset environment failed");
				}
			} else {
				perror("not enough arguments");
			}
			continue;
		}		

		// implement cd
		if (strcmp(_simpleCommands[i]->_arguments[0], "cd") == 0) {
			if (_simpleCommands[i]->_arguments[1]) {
				if (chdir(_simpleCommands[i]->_arguments[1]) < 0) {
					perror("cd");
				}
			} else {
				chdir(getenv("HOME"));
			}
			continue;
		}

		pid = fork();
		if (pid == -1) {
			perror(_simpleCommands[0]->_arguments[0]);
			perror(": fork\n");
			exit(2);
		}

		if (pid == 0) {
			//child

			if (strcmp(_simpleCommands[i]->_arguments[0], "printenv") == 0) {
				char **env = environ;
				while (*env) {
					puts(*env);
					env ++;
				}
			//continue;
				exit(0);
			}

			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			perror(": exec ");
			perror(_simpleCommands[i]->_arguments[0]);
			exit(2);
		}


	}
	

	dup2( defaultin, 0 );
	dup2( defaultout, 1 );
	dup2( defaulterr, 2 );
	close( defaultin );
	close( defaultout );
	close( defaulterr );

	if (_background == 0) {
		waitpid(pid, 0, 0);
	}

	clear();
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	//printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

void sigIntHandler(int i) {

}
void sigCHLDHandler(int i) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

main(int argc, char * const argv[])
{
	path = argv[0];
	struct sigaction signalAction;
    signalAction.sa_handler = sigIntHandler;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = SA_RESTART;

    int error = sigaction(SIGINT, &signalAction, NULL );
    if ( error ) {
        perror( "sigaction" );
        exit( -1 );
    }

    struct sigaction signalAction2;
    signalAction2.sa_handler = sigCHLDHandler;
    sigemptyset(&signalAction2.sa_mask);
    signalAction2.sa_flags = SA_RESTART;

    int error2 = sigaction(SIGCHLD, &signalAction2, NULL );
    if ( error2 ) {
        perror( "sigaction2" );
        exit( -1 );
    }

	Command::_currentCommand.prompt();
	yyparse();

}
