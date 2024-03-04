
CC := gcc
CFLAGS := -Wall

DEBUG_ALL ?= 0
DEBUG_VARS := DEBUG_PIPE DEBUG_CMD DEBUG_SHELL DEBUG_HIST DEBUG_JOB

DEBUG_FLAGS =
ifeq ($(DEBUG_ALL), 1)
	DEBUG_FLAGS += $(foreach flag,$(DEBUG_VARS),-D$(flag))
else
	DEBUG_FLAGS += $(foreach flag,$(DEBUG_VARS),$(if $($(flag)), -D$(flag)))
endif

SRCS = list.h pipe.h command.c command.h built-in.c built-in.h history.c history.h shell.c job.c job.h
OBJS = command.o history.o built-in.o job.o
.PHONY: debug default clean

default: cs5374_sh

job.o: job.c job.h command.h list.h pipe.h
	$(CC) $(CFLAGS) -c $< -o $@

command.o: command.c command.h list.h pipe.h built-in.c built-in.h
	$(CC) $(CFLAGS) -c $< -o $@

history.o: history.c history.h
	$(CC) $(CFLAGS) -c $< -o $@

built-in.o: built-in.c built-in.h pipe.h history.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

cs5374_sh: shell.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

debug: $(SRCS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $^ -o cs5374_sh

clean:
	rm *.o cs5374_sh
