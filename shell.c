#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "history.h"
#include "command.h"
#include "list.h"
#include "pipe.h"
#include "built-in.h"

static inline int do_command(struct command *cmd) {

	pid_t pid;

#ifdef DEBUG_SHELL
	fprintf(stderr, "(shell) debug: fork() for %s\n", cmd->argv[0]);
#endif

	pid = fork();
	if(pid == -1){
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}

	if(pid == 0){

		// child process

		if (pipe_dup2(&cmd->pipe_fd[0], STDIN_FILENO) == -1)
			exit(-1);
		if (pipe_dup2(&cmd->pipe_fd[1], STDOUT_FILENO) == -1)
			exit(-1);

		pipe_close(&cmd->unused_fd);

#ifdef DEBUG_SHELL
		fprintf(stderr, "(shell) debug: %*d: exec(%s)\n", 5, getpid(), cmd->argv[0]);
#endif
		execv(cmd->argv[0], cmd->argv);

		// if execv() returns, some error had occured
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(-1);

	}
	return 0;
}

int main(void) {

	char *input = NULL;
	size_t len = 0;
	ssize_t read_len;
	LIST_HEAD(cmd_list_head);
	struct list_head *node;
	struct command *cmd;
	int n_childs;
	int i;
	int builtin_idx;

	// TODO: Ctrl+c signal handler

	history_init();

	while (1) {

		putchar('$');
		read_len = getline(&input, &len, stdin);
		if (read_len < 0) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			continue;
		}
		// remove trailing newline
		if (input[read_len - 1] == '\n')
			input[read_len - 1] = '\0';

		history_push(input);

		command_list_parse(&cmd_list_head, input);
		if (list_empty(&cmd_list_head))
			continue;

		command_debug_list_print(&cmd_list_head);

		n_childs = 0;
		list_for_each(node, &cmd_list_head) {

			cmd = (struct command *)node;

			if (node->next != &cmd_list_head)
				if (pipe_make(cmd, (struct command *)node->next))
					break;

			command_debug_pipe(cmd);

			builtin_idx = is_builtin(cmd);
			if (builtin_idx) {
				if (do_builtin(builtin_idx, cmd) != 0) {
					// XXX: trigger exit
				}
			} else {
				if (do_command(cmd) == 0)
					n_childs++;
			}
			// close the pipe in parent,
			// so no extra pipes are brought
			// to the child in the next iter.
			pipe_close(&cmd->pipe_fd[0]);
			pipe_close(&cmd->pipe_fd[1]);
		}
#ifdef DEBUG_SHELL
		fprintf(stderr, "debug: n_childs = %d\n", n_childs);
#endif
		for (i = 0; i < n_childs; i++)
			if (wait(NULL) == -1)
				fprintf(stderr, "error: %s\n", strerror(errno));

		command_list_free(&cmd_list_head);

	}

	return 0;
}
