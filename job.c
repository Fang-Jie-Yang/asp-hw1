#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "job.h"
#include "list.h"
#include "command.h"
#include "pipe.h"

static void free_cmd_list(struct job *job);
static inline void job_debug_print(struct job *job);

// list is limited by _POSIX_ARG_MAX
// return NULL on *system* error
struct job *job_parse(char *s) {

	char *sub;
	struct command *cmd;
	struct job *job;
	int cnt = 0;
	int has_empty_cmd = 0;
	int parsing_err = 0;

	job = (struct job *)malloc(sizeof(job));
	if (job == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		return NULL;
	}
	job->pgid = -1;
	job->n_childs = -1;
	INIT_LIST_HEAD(&job->cmd_list);

	// use strsep because we don't allow contiguous '|'
	sub = strsep(&s, JOB_PIPE);
	while (sub != NULL) {

		cmd = command_parse(sub);

		if (cmd == NULL) {
			// *system* error when parsing command
			job_free(job);
			return NULL;
		}

		if (cmd->argc == 0) {
			// we encounterred a empty command
			has_empty_cmd = 1;
		}

		if (cmd->argc == -1) {
			// a command with too many arguments
			parsing_err = 1;
			break;
		}

		list_add_tail((struct list_head *)cmd, &job->cmd_list);
		cnt++;

		if (cnt >= _POSIX_ARG_MAX) {
			parsing_err = 1;
			break;
		}
		sub = strsep(&s, JOB_PIPE);
	}

	if (has_empty_cmd && cnt != 1)
		parsing_err = 1;

	if (parsing_err)
		fprintf(stderr, "error: parsing error\n");

	if (parsing_err || has_empty_cmd) {
		// we return a job with empty cmd list in both cases
		free_cmd_list(job);
	}

	return job;
}

// return -1 on error
int do_job(struct job *job) {

	int ret = 0;
	int child;
	int i;
	struct list_head *node;
	struct command *cmd;

	job_debug_print(job);

	job->n_childs = 0;

	list_for_each(node, &job->cmd_list) {

		cmd = (struct command *)node;

		if (node->next != &job->cmd_list) {
			if (pipe_make(cmd, (struct command *)node->next)) {
				ret = -1;
				break;
			}
		}

		child = do_command(cmd, &job->pgid);

		// close the pipe in parent,
		// so no extra pipes are brought
		// to the child in the next iter.
		if (pipe_close(&cmd->pipe_fd[0]) == -1) {
			ret = -1;
			break;
		}
		if (pipe_close(&cmd->pipe_fd[1]) == -1) {
			ret = -1;
			break;
		}

		if (child == -1) {
			ret = -1;
			break;
		} else {
			job->n_childs += child;
		}

	}
#ifdef DEBUG_JOB
	fprintf(stderr, "(job) debug: n_childs = %d\n", job->n_childs);
#endif
	for (i = 0; i < job->n_childs; i++) {
		if (wait(NULL) == -1) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			ret = -1;
		}
	}

	// put shell back to foreground
	if (tcsetpgrp(STDIN_FILENO, getpid()) == -1) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		ret = -1;
	}

	return ret;
}

static void free_cmd_list(struct job *job) {

	struct list_head *node;
	struct list_head *tmp;

	if (job == NULL) {
		fprintf(stderr, "warning: free_cmd_list() on NULL ptr\n");
		return;
	}

	node = job->cmd_list.next;
	while (node != &job->cmd_list) {
		tmp = node->next;
		command_free((struct command **)&node);
		node = tmp;
	}
	INIT_LIST_HEAD(&job->cmd_list);
	return;
}

void job_free(struct job *job) {

	if (job == NULL) {
		fprintf(stderr, "warning: job_free() on NULL ptr\n");
		return;
	}

	free_cmd_list(job);
	free(job);
	return;
}

static inline void job_debug_print(struct job *job) {
#ifdef DEBUG_JOB
	struct list_head *node;

	if (job == NULL) {
		fprintf(stderr, "(job) debug: job_debug_print() on NULL ptr\n");
		return;
	}

	list_for_each(node, &job->cmd_list) {
		// XXX: dependency on DEBUG_CMD
		command_debug_print((struct command *)node);
	}
#endif
	return;
}
