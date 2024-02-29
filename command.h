#ifndef COMMAND_H
#define COMMAND_H 

#define COMMAND_PIPE  "|"
#define COMMAND_DELIM " "

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

// the number of entries in the returned list
// will be limited by _POSIX_ARG_MAX
void command_list_parse(struct list_head *cmd_list, char *s);
//void command_free(struct command **cmdp);
void command_list_free(struct list_head *cmd_list);
void command_debug_print(struct command *cmd);
void command_debug_list_print(struct list_head *cmd_list);

#endif
