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

	while (token != NULL) {
		//fprintf(stderr, "-- debug: %s\n", token);
		res->argv[argc] = token;
		argc++;

		if (argc >= _POSIX_ARG_MAX) {
			fprintf(stderr, "error: too many arguments\n");
			goto fail;
		}

		token = strtok(NULL, COMMAND_DELIM);
	}

	res->argv[argc] = (char *)NULL;

	return res;

fail:
	free(res);
	return NULL;
}

// the number of entries in the returned array
// will be limited by _POSIX_ARG_MAX
// return a null-terminated list
void command_list_parse(struct list_head *cmd_list, char *s) {

	char *sub;
	int cnt = 0;
	struct command *cmd;

	if (*s == '\0') {
		return;
	}

	// DEBUG: make sure the given list_head is empry
	if (!list_empty(cmd_list)) {
		fprintf(stderr, "command_list_parse: not empty list\n");
		INIT_LIST_HEAD(cmd_list);
	}

	// use strsep because we don't allow contiguous '|'
	sub = strsep(&s, COMMAND_PIPE);
	while (sub != NULL) {
		//fprintf(stderr, "debug: %s\n", sub);
		cmd = command_parse(sub);

		if (cmd == NULL) {
			// we encounter a empty command
			fprintf(stderr, "error: parsing error\n");
			goto fail;
		}

		list_add_tail((struct list_head *)cmd, cmd_list);
		cnt++;

		if (cnt >= _POSIX_ARG_MAX) {
			fprintf(stderr, "error: too many arguments\n");
			goto fail;
		}
		sub = strsep(&s, COMMAND_PIPE);
	}
	return;

fail:
	command_list_free(cmd_list);	
}


static void command_free(struct command **cmdp) {
	// XXX: sigmask?
	free(*cmdp);
	*cmdp = NULL;
	return;
}

void command_list_free(struct list_head *cmd_list) {

	struct list_head *node;
	struct list_head *tmp;

	if (cmd_list == NULL) {
		return;
	}

	node = cmd_list->next;
	while (node != cmd_list) {
		tmp = node->next;
		command_free((struct command **)&node);
		node = tmp;
	}
	INIT_LIST_HEAD(cmd_list);
	return;
}

void command_print(struct command *cmd) {

	int i;

	if (cmd == NULL) {
		return;
	}

	for (i = 0; ; i++) {
		if (cmd->argv[i] == NULL) {
			break;
		}
		fprintf(stderr, "==== argv[%d] : %s\n", i, cmd->argv[i]);
	}
	return;
}

void command_list_print(struct list_head *cmd_list) {

	struct list_head *node;

	if (cmd_list == NULL) {
		return;
	}

	list_for_each(node, cmd_list) {
		command_print((struct command *)node);
	}
	return;
}
