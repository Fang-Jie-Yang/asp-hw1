#ifndef BUILT_IN_H
#define BUILT_IN_H

#include "history.h"
#include "command.h"


struct builtin_struct {
	const char *cmd;
	int (*fn)(struct command *);
};

// return built-in idx for built-ins, 0 for non-built-in commands
int is_builtin(const struct command *cmd);
int do_builtin(int idx, struct command *cmd);

#endif
