#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "history.h"
#include "job.h"

int main(void) {

	char *input = NULL;
	size_t len = 0;
	ssize_t read_len;
	struct job *job;

	// Note that we didn't check whether we are in interacive mode
	// and that we are in foreground

	// create a new process group
	if (setpgrp() == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(-1);
	}
	// control the terminal
	if (tcsetpgrp(STDIN_FILENO, getpid()) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(-1);
	}

	// TODO: Ctrl+c signal handler
	// Note that we need to make sure that
	// we do not read/write from the terminal
	// when we are in background
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	history_init();

	while (1) {

		putchar('$');
		read_len = getline(&input, &len, stdin);
		if (read_len < 0) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			break;
		}
		// remove trailing newline
		if (input[read_len - 1] == '\n')
			input[read_len - 1] = '\0';

		// XXX: handle error
		history_push(input);

		// XXX: differentiate between empty job & failure
		job = job_parse(input);
		if (job == NULL)
			continue;

		if (do_job(job) == -1) {
			break;
		}

		job_free(job);

	}

	return 0;
}
