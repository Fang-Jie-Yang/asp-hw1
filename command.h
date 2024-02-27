#ifndef COMMAND_H
#define COMMAND_H 

#define COMMAND_PIPE  "|"
#define COMMAND_DELIM " "

struct command {
	char *pathname;
	// null-terminated, 
	char *argv[];
	// char *envp[];
};

// the "cache" for storing parsing outcome
// some we don't have to alloc everytime
extern struct command **command_list_cache;

// the number of entries in the returned list
// will be limited by _POSIX_ARG_MAX
struct command **command_list_parse(char *s);
void command_free(struct command *list[]);
void command_print(struct command *cmd);
void command_list_print(struct command **cmd_list);

#endif
