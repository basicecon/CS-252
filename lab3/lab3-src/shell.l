
/*
 *
 * CS-252 Spring 2014
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "y.tab.h"
#include "subshell.h"


////////////  Start added code ///////////

extern "C" char * read_line();

int mygetc(FILE * f) {
  static char *p;
  char ch;

  if (!isatty(0)) {
    // stdin is not a tty. Call real getc
    return getc(f);
  }

  // stdin is a tty. Call our read_line.

  if (p==NULL || *p == 0) {
    char * s = read_line();
    p = s;
  }

  ch = *p;
  p++;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)

/////////// End added code ///////////


static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
    unput(c);
}

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
		}

">>" 	{
		return GREATGREAT;
		}

">&" 	{
		return GREATAMPERSAND;
		}

">>&" 	{
		return GREATGREATAMPERSAND;
		}

"<"	 	{
		return LESS;
		}

"|"  	{
		return PIPE;
		}

"&" 	{
		return AMPERSAND;
		}

`[^\n]*` {
		char *text = yytext + 1;
		text[strlen(text)-1] = 0;
		 /* yyrestart(stdin); */
		subshell(text);
	}		

\"[^\"\n]*\" {
		char *text = yytext + 1;
		text[strlen(text)-1] = 0;
		yylval.string_val = strdup(text);
		return WORD;
	}		


(\\.|[^ \t\n\|\>\<\&])+  {
		/* Assume that file names have only alpha chars */
		char *new_yytext = (char*)malloc(strlen(yytext));
		char *head = new_yytext;
		while (*yytext) {
			if (*yytext != '\\') {
				*new_yytext = *yytext;
				new_yytext ++;
			} else {
				yytext ++;
				*new_yytext = *yytext;
				new_yytext ++;
			}
			yytext ++;
		}
		*new_yytext = '\0';

		yylval.string_val = strdup(head);
		free(head);
		return WORD;
	}	
