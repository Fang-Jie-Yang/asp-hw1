
command: command.h command.c list.h
	gcc -c -o command.o command.c

history: history.h history.c
	gcc -c -o history.o history.c

shell: shell.c history command 
	gcc -o cs5374_sh shell.c history.o command.o

