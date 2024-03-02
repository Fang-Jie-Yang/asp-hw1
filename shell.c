#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "history.h"
#include "job.h"

volatile sig_atomic_t exit_flag = 0;

static void sigint_handler(int sig) {
	exit_flag = 1;
	return;
}

static inline int register_sa(int sig, void (*handler)(int)) {
	struct sigaction sa;
	if (sigemptyset(&sa.sa_mask) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	if (sigaction(sig, &sa, NULL) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
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
	if (register_sa(SIGTTIN, SIG_IGN) == -1 ||
	    register_sa(SIGTTOU, SIG_IGN) == -1 ||
	    register_sa(SIGINT, sigint_handler) == -1) {
		exit(-1);
	}

	if (history_init() == -1)
		exit(-1);

	while (exit_flag != 1) {

		putchar('$');
		read_len = getline(&input, &len, stdin);
		if (read_len < 0) {
			// we allow the shell to be terminated by Ctrl+c
			if (errno != EINTR) 
				fprintf(stderr, "error: %s\n", strerror(errno));
			break;
		}
		// remove trailing newline
		if (input[read_len - 1] == '\n')
			input[read_len - 1] = '\0';

		if (history_push(input) == -1)
			break;

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

	// *job is free'd before exiting the loop
	history_free();
	if (input != NULL)
		free(input);
	
	return -1;
}
