#ifndef COMMAND_H
#define COMMAND_H 

#define COMMAND_DELIM " "

#include <sys/types.h>

#include "list.h"

struct command {
	struct list_head list_head;
	int pipe_fd[2];
	int unused_fd;
	// null-terminated, 
	int argc;
	char *argv[];
	// char *envp[];
};

struct command *command_parse(char *s);
// return the number of child fork()'d 
// => 0 on built-in command
// => 1 on extern command
// => -1 on error
int do_command(struct command *cmd, pid_t *pgid);
void command_free(struct command **cmdp);
void command_debug_print(struct command *cmd);

#endif
