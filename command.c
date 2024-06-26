#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

#include "command.h"
#include "pipe.h"
#include "built-in.h"

static inline void command_debug_pipe(struct command *cmd);

// parse a space-separated string into struct command
// return NULL on *system* error
struct command *command_parse(char *s) {

	char *token;
	int argc = 0;
	struct command *tmp;
	struct command *res;

	tmp = (struct command *)malloc(sizeof(struct command) + sizeof(char *) * _POSIX_ARG_MAX);
	if (tmp == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return NULL;
	}
	tmp->pipe_fd[0] = -1;
	tmp->pipe_fd[1] = -1;
	tmp->unused_fd = -1;
	tmp->argc = -1;

	token = strtok(s, COMMAND_DELIM);
	while (token != NULL) {

		tmp->argv[argc] = token;
		argc++;

		if (argc >= _POSIX_ARG_MAX) {
			fprintf(stderr, "error: too many arguments\n");
			argc = -1;
			break;
		}

		token = strtok(NULL, COMMAND_DELIM);
	}

	tmp->argc = argc;
	tmp->argv[argc] = (char *)NULL;
	res = (struct command *)realloc(tmp, sizeof(struct command) + sizeof(char *) * (argc + 1));
	if (res == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		free(tmp);
		return NULL;
	}
	return res;
}

// return the number of child fork()'d 
// => 0 on built-in command
// => 1 on extern command
// => -1 on error
int do_command(struct command *cmd, pid_t *pgid) {

	int builtin_idx = -1;
	pid_t pid;

	command_debug_pipe(cmd);

	builtin_idx = is_builtin(cmd);
	if (builtin_idx != -1) {
		if (do_builtin(builtin_idx, cmd) != 0)
			return -1;
		return 0;
	}

#ifdef DEBUG_CMD
	fprintf(stderr, "(cmd) debug: fork() for %s\n", cmd->argv[0]);
#endif

	pid = fork();
	if(pid == -1){
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}

	if(pid == 0){

		// child process
		// XXX: (fail before execv()) free resources before termination?

		if (*pgid == -1) {
			// we are the leader
			*pgid = getpid();
			if (setpgrp() == -1) {
				fprintf(stderr, "error: %s\n", strerror(errno));
				exit(-1);
			}
		} else {
			if (setpgid(0, *pgid) == -1) {
				fprintf(stderr, "error: %s\n", strerror(errno));
				exit(-1);
			}
		}

		// set the job to foreground
		if (tcsetpgrp(STDIN_FILENO, *pgid) == -1) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			exit(-1);
		}

		// restore signal handler, since we have ignored them in main()	
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);

		if (pipe_dup2(&cmd->pipe_fd[0], STDIN_FILENO) == -1)
			exit(-1);
		if (pipe_dup2(&cmd->pipe_fd[1], STDOUT_FILENO) == -1)
			exit(-1);

		if (pipe_close(&cmd->unused_fd) == -1) 
			exit(-1);

#ifdef DEBUG_CMD
		fprintf(stderr, "(cmd) debug: %*d: exec(%s)\n", 5, getpid(), cmd->argv[0]);
#endif
		execv(cmd->argv[0], cmd->argv);

		// if execv() returns, some error had occured
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(-1);

	} else {

		// parent process

		if (*pgid == -1) {
			// this child is the leader
			*pgid = pid;
		}

		// set child pid for child to avoid race cond
		if (setpgid(pid, *pgid) == -1) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return -1;
		}

	}
	return 1;
}


void command_free(struct command **cmdp) {
	free(*cmdp);
	*cmdp = NULL;
	return;
}


void command_debug_print(struct command *cmd) {
#ifdef DEBUG_CMD
	int i;

	if (cmd == NULL) 
		return;

	for (i = 0; ; i++) {
		if (cmd->argv[i] == NULL)
			break;
		fprintf(stderr, "argv[%d] : %s\n", i, cmd->argv[i]);
	}
	fprintf(stderr, "\n");
#endif
	return;
}

static inline void command_debug_pipe(struct command *cmd) {
#ifdef DEBUG_CMD
	char debug_info[1024] = "\0";
	char line[256] = "\0";

	snprintf(line, 256, "(shell) debug: command: %s\n", cmd->argv[0]);
	strncat(debug_info, line, 1024);
	snprintf(line, 256, "               |-- pipe  fds: %d %d\n", cmd->pipe_fd[0], cmd->pipe_fd[1]);
	strncat(debug_info, line, 1024);
	snprintf(line, 256, "               '-- unused fd: %d \n", cmd->unused_fd);
	strncat(debug_info, line, 1024);
	fprintf(stderr, "%s", debug_info);
#endif
	return;
}
