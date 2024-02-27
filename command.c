#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "command.h"

struct command **command_list_cache = NULL;

// parse a space-separated string into struct command
static struct command *command_parse(char *s) {

	char *token;
	int argc = 0;
	struct command *res;

	token = strtok(s, COMMAND_DELIM);
	if (token == NULL) {
		// an empty command
		//fprintf(stderr, "error: parsing error\n");
		return NULL;
	}

	res = (struct command *)malloc(sizeof(struct command) + sizeof(char *) * _POSIX_ARG_MAX);
	if (res == NULL) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		return NULL;
	}
	res->pathname = token;

	while (token != NULL) {
		fprintf(stderr, "-- debug: %s\n", token);
		res->argv[argc] = token;
		argc++;

		if (argc >= _POSIX_ARG_MAX) {
			fprintf(stderr, "error: too many arguments\n");
			return NULL;
		}

		token = strtok(NULL, COMMAND_DELIM);
	}

	res->argv[argc] = (char *)NULL;

	return res;
}

// the number of entries in the returned array
// will be limited by _POSIX_ARG_MAX
// return a null-terminated list
struct command **command_list_parse(char *s) {

	char *sub;
	int cnt = 0;
	struct command *cmd;

	if (command_list_cache == NULL) {
		command_list_cache = malloc(sizeof(struct command *) * _POSIX_ARG_MAX);
		if (command_list_cache == NULL) {
			fprintf(stderr, "error: %s\n", strerror(errno));
		}
	}

	if (*s == '\0') {
		return NULL;
	}

	// use strsep because we don't allow contiguous '|'
	sub = strsep(&s, COMMAND_PIPE);
	while (sub != NULL) {
		fprintf(stderr, "debug: %s\n", sub);
		cmd = command_parse(sub);

		if (cmd == NULL) {
			// we encounter a empty command
			fprintf(stderr, "error: parsing error\n");
			return NULL;
		}

		command_list_cache[cnt] = cmd;
		cnt++;

		if (cnt >= _POSIX_ARG_MAX) {
			fprintf(stderr, "error: too many arguments\n");
			return NULL;
		}
		sub = strsep(&s, COMMAND_PIPE);
	}
	command_list_cache[cnt] = (struct command *)NULL;

	return command_list_cache;
}


void command_free(struct command **cmdp) {
	// XXX: sigmask?
	free(*cmdp);
	*cmdp = NULL;
	return;
}

void command_list_free(struct command **cmd_list) {

	int i;

	for (i = 0; ; i++) {
		if (cmd_list[i] == NULL) {
			break;
		}
		command_free(&cmd_list[i]);
	}
	return;
}

void command_print(struct command *cmd) {

	int i;

	if (cmd == NULL) {
		return;
	}

	if (cmd->pathname == NULL) {
		fprintf(stderr, "==== NULL pathname!\n");
		return;
	}
	fprintf(stderr, "==== pathname: %s\n", cmd->pathname);
	for (i = 0; ; i++) {
		if (cmd->argv[i] == NULL) {
			break;
		}
		fprintf(stderr, "==== argv    : %s\n", cmd->argv[i]);
	}
	return;
}

void command_list_print(struct command **cmd_list) {

	int i;

	if (cmd_list == NULL) {
		return;
	}

	for (i = 0; ; i++) {
		if (cmd_list[i] == NULL) {
			break;
		}
		command_print(cmd_list[i]);
	}
	return;
}
