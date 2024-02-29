#ifndef PIPE_H
#define PIPE_H

#include <unistd.h>

#include "command.h"

// return -1 on error
static inline int pipe_make(struct command *cmd, struct command *next) {

	int fds[2];

	if (pipe(fds) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}

	cmd->pipe_fd[1]  = fds[1];
	cmd->unused_fd = fds[0];

	next->pipe_fd[0] = fds[0];

	return 0;
}

static inline void pipe_close(int *pipefd) {

	fprintf(stderr, "debug: %*d: close(%d)\n", 5, getpid(), *pipefd);

	if (*pipefd == -1)
		return;

	if (close(*pipefd) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		fprintf(stderr, "debug: close() on %d\n", *pipefd);
		// XXX: I think we have to abort in this case
		// TODO: free resources
		exit(-1);
	}
	*pipefd = -1;
}

// return -1 on error, *pipefd is closed either success or fail
static inline int pipe_dup2(int *pipefd, int newfd) {

	int ret = 0;

	fprintf(stderr, "debug: %*d: dup2 (%d) => (%d)\n", 5, getpid(), *pipefd, newfd);

	if (*pipefd == -1)
		return 0;

	ret = dup2(*pipefd, newfd);
	if (ret == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
	}
	pipe_close(pipefd);
	return ret;
}

#endif
