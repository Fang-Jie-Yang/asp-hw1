#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "history.h"
#include "command.h"

int main(void) {

	char *input = NULL;
	size_t len = 0;
	ssize_t read_len;
	struct history *hist;
	struct command **cmd_list;
	int i;

	// TODO: Ctrl+c signal handler

	// init history
	hist = history_new();

	// main loop
	while (1) {

		putchar('$');
		// get a "line" of input
		read_len = getline(&input, &len, stdin);
		if (read_len < 0) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			// XXX: continue or break?
			continue;
		}
		// remove trailing newline
		if (input[read_len - 1] == '\n') {
			input[read_len - 1] = '\0';
		}
		history_push(hist, input);

		// TODO: split line into cmds by "|"
		cmd_list = command_list_parse(input);

		// for each cmd, split by " "
			// handle cmd+arg
		command_list_print(cmd_list);
		history_show(hist, 10);

	}

	return 0;
}
