#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "built-in.h"
#include "pipe.h"
#include "history.h"
#include "command.h"

extern int exit_flag;

static int do_cd(struct command *cmd);
static int do_history(struct command *cmd);
static int do_exit(struct command *cmd);

static struct builtin_struct builtins[] = {
	{ "cd", 		do_cd 		},
	{ "history", 	do_history 	},
	{ "exit",		do_exit		},
	{ NULL,			NULL		},
};

int is_builtin(const struct command *cmd) {
	int i = 0;
	while (builtins[i].cmd != NULL) {
		if (strcmp(builtins[i].cmd, cmd->argv[0]) == 0) {
			return i;
		}
		i++;
	}
	return -1;
}

int do_builtin(int idx, struct command *cmd) {
	// note that we don't close "unused_fd" 
	// because we are in parent, 
	// the "unused_fd" is actually useful 
	// for the next command in the job
	return builtins[idx].fn(cmd);
}

static int do_cd(struct command *cmd) {

	if (cmd->argc != 2) {
		fprintf(stderr, "error: cd: invalid arguments\n");
		return 0;
	}

	if (chdir(cmd->argv[1]) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return 0;
	}

	return 0;
}

static int do_history(struct command *cmd) {

	char *endptr;
	long int n;
	FILE *out_stream;

	if (cmd->argc > 2) {
		fprintf(stderr, "error: history: invalid arguments\n");
		return 0;
	}

	// allowing history output to be piped
	if (cmd->pipe_fd[1] != -1) {
		out_stream = fdopen(cmd->pipe_fd[1], "w");
		if (out_stream == NULL) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return -1;
		}
	} else {
		out_stream = stdout;
	}

	if (cmd->argc == 1) {
		history_show(out_stream, 10);
		return 0;
	}

	if (strcmp(cmd->argv[1], "-c") == 0) {
		history_clear();
		return 0;
	} 

	n = strtol(cmd->argv[1], &endptr, 10);

	if (*endptr != '\0') {
		fprintf(stderr, "error: invalid number\n");
		return 0;
	} 
	history_show(out_stream, n);

	return 0;
}

static int do_exit(struct command *cmd) {

	if (cmd->argc != 1) {
		fprintf(stderr, "error: exit: invalid arguments\n");
		return 0;
	}

	exit_flag = 1;

	return 0;
}


