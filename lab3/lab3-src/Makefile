
#Use GNU compiler
cc = gcc -g
CC = g++ -g

LEX=lex
YACC=yacc

all: shell cat_grep ctrl-c regular

tty-raw-mode.o: tty-raw-mode.c
	$(cc) -c tty-raw-mode.c

read-line.o: read-line.c
	$(cc) -c read-line.c

subshell.o: subshell.cc
	$(CC) -c subshell.cc	

lex.yy.o: shell.l 
	$(LEX) shell.l
	$(CC) -c lex.yy.c

y.tab.o: shell.y
	$(YACC) -d shell.y
	$(CC) -c y.tab.c

command.o: command.cc
	$(CC) -c command.cc

wildcarding.o: wildcarding.cc
	$(CC) -c wildcarding.cc

shell: y.tab.o lex.yy.o command.o wildcarding.o subshell.o tty-raw-mode.o read-line.o
	$(CC) -o shell lex.yy.o y.tab.o command.o wildcarding.o subshell.o tty-raw-mode.o read-line.o -lfl

cat_grep: cat_grep.cc
	$(CC) -o cat_grep cat_grep.cc

ctrl-c: ctrl-c.cc
	$(CC) -o ctrl-c ctrl-c.cc

regular: regular.cc
	$(CC) -o regular regular.cc 

clean:
	rm -f lex.yy.c y.tab.c  y.tab.h shell ctrl-c regular cat_grep *.o
