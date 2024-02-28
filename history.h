#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX_ENTRY 99999
#define HISTORY_NUM_WIDTH 5
struct history {
	char *buf[HISTORY_MAX_ENTRY];
	int cnt;
};

// global history structure
extern struct history *hist;

// return -1 on error
int history_init(void);
int history_push(const char *s);
// clear history buffer and free up strings stored in it
void history_clear();
void history_show(long int n);
void history_free();

#endif
