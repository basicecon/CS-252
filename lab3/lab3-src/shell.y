
/*
 * CS-252 Spring 2014
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE GREATGREAT GREATAMPERSAND GREATGREATAMPERSAND LESS PIPE AMPERSAND

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include <string.h>
#include "command.h"
#include "wildcarding.h"
void yyerror(const char * s);
int yylex();

%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	pipe_list iomodifier_list background_opt NEWLINE {
		//printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

background_opt:
	AMPERSAND {
		Command::_currentCommand._background = 1;
	}
	| /*empty*/
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

pipe_list:
	pipe_list PIPE command_and_args
	| command_and_args
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               //printf("   Yacc: insert argument \"%s\"\n", $1);

	       //Command::_currentSimpleCommand->insertArgument( $1 );
			expandWildCard("", $1);
	}
	;

command_word:
	WORD {
               //printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_list:
	iomodifier_list iomodifier_opt
	| /*empty*/
	;

iomodifier_opt:
	GREAT WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2);
		if (Command::_currentCommand._outFile) {
			perror("Ambiguous output redirect\n");
			exit(2);
		}
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._isOutAppend = 0;
	}
	| GREATGREAT WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2);
		if (Command::_currentCommand._outFile) {
			perror("Ambiguous output redirect\n");
			exit(2);
		}
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._isOutAppend = 1;
	}
	| GREATAMPERSAND WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2);
		if (Command::_currentCommand._outFile) {
			perror("Ambiguous output redirect\n");
			exit(2);
		}
		if (Command::_currentCommand._errFile) {
			perror("Ambiguous output redirect\n");
			exit(2);
		}
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = strdup($2);
		Command::_currentCommand._isOutAppend = 0;
		Command::_currentCommand._isErrAppend = 0;
	}
	| GREATGREATAMPERSAND WORD {
		//printf("   Yacc: insert output \"%s\"\n", $2);
		if (Command::_currentCommand._outFile) {
			perror("Ambiguous output redirect\n");
			exit(2);
		}
		if (Command::_currentCommand._errFile) {
			perror("Ambiguous output redirect\n");
			exit(2);
		}
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = strdup($2);
		Command::_currentCommand._isOutAppend = 1;
		Command::_currentCommand._isErrAppend = 1;
	}
	| LESS WORD {
		//printf("   Yacc: insert input \"%s\"\n", $2);
		if (Command::_currentCommand._inputFile) {
			perror("Ambiguous input redirect\n");
			exit(2);
		}
		Command::_currentCommand._inputFile = $2;
	}
	;



%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
