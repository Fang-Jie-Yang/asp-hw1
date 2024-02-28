#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "built-in.h"
#include "pipe.h"
#include "history.h"
#include "command.h"

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
	return 0;
}

int do_builtin(int idx, struct command *cmd) {
	return builtins[idx].fn(cmd);
}

// we do built-ins in parent process,
// so we actually don't have to free pipes before handling commands
// if we want to support pipe for built-ins.
// But we do have to free them after the command is finished
static void builtin_free_pipes(struct command *cmd) {
	pipe_close(&cmd->pipe_fd[0]);	
	pipe_close(&cmd->pipe_fd[1]);	
}

static int do_cd(struct command *cmd) {

	int ret = 0;

	if (cmd->argc != 2) {
		fprintf(stderr, "error: cd: invalid arguments\n");
		ret = -1;
		goto end;
	}

	if (chdir(cmd->argv[1]) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		ret = -1;
	}

end:
	builtin_free_pipes(cmd);
	return ret;
}

static int do_history(struct command *cmd) {

	int ret = 0;
	char *endptr;
	long int n;

	if (cmd->argc > 2) {
		fprintf(stderr, "error: cd: invalid arguments\n");
		ret = -1;
		goto end;
	}

	if (cmd->argc == 1) {
		history_show(10);
		goto end;
	}

	if (strcmp(cmd->argv[1], "-c") == 0) {
		history_clear();
		goto end;
	} 

	n = strtol(cmd->argv[1], &endptr, 10);

	if (*endptr != '\0') {
		fprintf(stderr, "error: %s\n", strerror(errno));
		ret = -1;
		goto end;	
	} 
	history_show(n);	

end:
	builtin_free_pipes(cmd);
	return ret;
}

static int do_exit(struct command *cmd) {

	int ret = 0;

	if (cmd->argc != 1) {
		fprintf(stderr, "error: cd: invalid arguments\n");
		ret = -1;
		goto end;
	}

	// exit

end:
	builtin_free_pipes(cmd);
	return ret;
}


