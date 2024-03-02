#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#include "list.h"
#include "command.h"

#define JOB_PIPE  "|"

struct job {
	pid_t pgid;
	int n_childs;
	struct list_head cmd_list;
};

// the number of entries in the returned list
// will be limited by _POSIX_ARG_MAX
struct job *job_parse(char *s, int *err);
// return -1 on error
int do_job(struct job *job);
void job_free(struct job *job);
#endif
