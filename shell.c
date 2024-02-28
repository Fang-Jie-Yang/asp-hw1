#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "history.h"
#include "command.h"
#include "list.h"

// return -1 on error
static inline int pipe_make(struct command *cmd, struct command *next) {

	int fds[2];

	if (pipe(fds) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}

	cmd->pipe_fd[1]  = fds[1];
	next->pipe_fd[0] = fds[0];

	cmd->unused_fd[0]  = fds[0];
	next->unused_fd[1] = fds[1];

	return 0;
}

static inline void pipe_close(int *pipefd) {
	if (*pipefd == -1)
		return;
	if (close(*pipefd) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		// XXX: I think we have to abort in this case
		// TODO: free resources
		exit(-1);
	}
	*pipefd = -1;
}

// return -1 on error, *pipefd is closed either success or fail
static inline int pipe_dup2(int *pipefd, int newfd) {
	int ret;
	if (*pipefd == -1)
		return 0;
	ret = dup2(*pipefd, newfd);
	if (ret == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	pipe_close(pipefd);
	return ret;
}

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
		execlp("ls","ls",NULL);
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
	struct history *hist;
	LIST_HEAD(cmd_list_head);
	struct list_head *cmd;

	// TODO: Ctrl+c signal handler

	hist = history_new();

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
		history_push(hist, input);

		command_list_parse(&cmd_list_head, input);
		if (list_empty(&cmd_list_head)) {
			continue;
		}

		command_list_print(&cmd_list_head);
		list_for_each(cmd, &cmd_list_head) {
			if (cmd->next != &cmd_list_head) {
				if (pipe_make((struct command *)cmd, (struct command *)cmd->next)) {
					break;
				}
			}
			if (do_command((struct command *)cmd)) {
				break;
			}
		}

		command_list_free(&cmd_list_head);
		history_show(hist, 10);

	}

	return 0;
}
