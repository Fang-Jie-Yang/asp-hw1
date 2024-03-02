#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "history.h"
#include "job.h"

volatile sig_atomic_t exit_flag = 0;

void catch_sigint(int sig) {
	exit_flag = 1;
	signal(sig, catch_sigint);
}

int main(void) {

	char *input = NULL;
	size_t len = 0;
	ssize_t read_len;
	struct job *job;
	int err = 0;

	// Note that we didn't check:
	// 1. whether we are in interacive mode
	// 2. we are in foreground

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

	// ignore job control signals
	if (signal(SIGTTIN, SIG_IGN) == SIG_ERR ||
	    signal(SIGTTOU, SIG_IGN) == SIG_ERR ||
	    signal(SIGINT, catch_sigint) == SIG_ERR) {

		fprintf(stderr, "error: %s\n", strerror(errno));
		exit(-1);
	}

	if (history_init() == -1)
		exit(-1);

	while (exit_flag != 1) {

		putchar('$');
		read_len = getline(&input, &len, stdin);
		if (read_len < 0) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			break;
		}
		// remove trailing newline
		if (input[read_len - 1] == '\n')
			input[read_len - 1] = '\0';

		if (history_push(input) == -1)
			break;

		// XXX: differentiate between empty job & failure
		job = job_parse(input, &err);
		if (job == NULL) {
			if (err != 0)
				break;
			// just a empty job
			continue;
		}

		if (do_job(job) == -1)
			exit_flag = 1;

		job_free(job);

	}

#ifdef DEBUG_SHELL
	fprintf(stderr, "(shell) debug: exiting\n");
#endif
	// *job is free'd before exiting the loop
	history_free();
	
	return -1;
}
