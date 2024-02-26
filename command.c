#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "command.h"

struct command command_list_cache[] = NULL;

static void command_parse() {
}
// the number of entries in the returned array
// will be limited by _POSIX_ARG_MAX
// return a null-terminated list
struct command command_list_parse[](const char *s) {

	char *token;
	int cnt = 0;

	if (command_list_cache == NULL) {
		command_list_cache = malloc(sizeof(struct command *) * _POSIX_ARG_MAX);
		if (command_list_cache == NULL) {
			fprintf(stderr, "error: %s\n", strerror(errno));
		}
	}

	token = strtok(s, COMMAND_PIPE);
	while (token != NULL) {
		// XXX: double pipe? "a || b"
		fprintf(stderr, "debug: %s\n", token);
		
		token = strtok(NULL, COMMAND_PIPE);
	}


	return command_list_cache;
}


void command_free(struct command *list[]);
