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

	pid = fork();
	if(pid == -1){
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}

	// child
	if(pid == 0){
		if (pipe_dup2(&cmd->pipe_fd[0], STDOUT_FILENO)) {
			exit(-1);
		}
		if (pipe_dup2(&cmd->pipe_fd[1], STDIN_FILENO)) {
			exit(-1);
		}
		pipe_close(&cmd->unused_fd[0]);
		pipe_close(&cmd->unused_fd[1]);
		execv(cmd->argv[0], cmd->argv);
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(-1);
	}

	// parent
	pipe_close(&cmd->pipe_fd[0]);
	pipe_close(&cmd->pipe_fd[1]);
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
		if (input[read_len - 1] == '\n') {
			input[read_len - 1] = '\0';
		}
		history_push(input);

		command_list_parse(&cmd_list_head, input);
		if (list_empty(&cmd_list_head)) {
			continue;
		}

		command_list_print(&cmd_list_head);

		n_childs = 0;
		list_for_each(node, &cmd_list_head) {

			cmd = (struct command *)node;

			if (node->next != &cmd_list_head) {
				if (pipe_make(cmd, (struct command *)node->next)) {
					break;
				}
			}

			builtin_idx = is_builtin(cmd);
			fprintf(stderr, "debug: built-in id= %d\n", builtin_idx);
			if (builtin_idx) {
				if (do_builtin(builtin_idx, cmd)) {
					continue;
				}
			} else {
				if (do_command(cmd)) {
					break;
				} else {
					n_childs++;
				}
			}
		}

		for (i = 0; i < n_childs; i++) {
			if (wait(NULL) == -1) {
				fprintf(stderr, "error: %s\n", strerror(errno));
			}
		}

		command_list_free(&cmd_list_head);
		//history_show(10);

	}

	return 0;
}
