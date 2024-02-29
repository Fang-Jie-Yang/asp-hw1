
SHELL := /bin/bash
CC = gcc
CFLAGS = -Wall

DEBUG_FLAGS = 
DEBUG_PIPE ?= 0
DEBUG_CMD ?= 0
DEBUG_SHELL ?= 0
DEBUG_HIST ?= 0
ifeq ($(DEBUG_PIPE), 1)
	DEBUG_FLAGS += -DDEBUG_PIPE
endif
ifeq ($(DEBUG_CMD), 1)
	DEBUG_FLAGS += -DDEBUG_CMD
endif
ifeq ($(DEBUG_SHELL), 1)
	DEBUG_FLAGS += -DDEBUG_SHELL
endif
ifeq ($(DEBUG_HIST), 1)
	DEBUG_FLAGS += -DDEBUG_HIST
endif

SRCS = list.h pipe.h command.c command.h built-in.c built-in.h history.c history.h shell.c
OBJS = command.o history.o built-in.o

command.o: command.c command.h list.h
	$(CC) $(CFLAGS) -c $< -o $@

history.o: history.c history.h
	$(CC) $(CFLAGS) -c $< -o $@

built-in.o: built-in.c built-in.h pipe.h history.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

cs5374_sh: shell.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: debug
debug: $(SRCS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -o cs5374_sh
