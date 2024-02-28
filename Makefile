
CC = gcc
CFLAGS = -Wall

OBJS = command.o history.o built-in.o

command.o: command.c command.h list.h
	$(CC) $(CFLAGS) -c $< -o $@

history.o: history.c history.h
	$(CC) $(CFLAGS) -c $< -o $@

built-in.o: built-in.c built-in.h pipe.h history.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

cs5374_sh: shell.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

