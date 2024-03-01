#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "history.h"
#include "job.h"

int main(void) {

	char *input = NULL;
	size_t len = 0;
	ssize_t read_len;
	struct job *job;
	int i;

	// TODO: Ctrl+c signal handler

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
