#ifndef HISTORY_H
#define HISTORY_H

#define MAX_HISTORY_ENTRY 99999
struct history {
	char *buf[MAX_HISTORY_ENTRY];
	int cnt;
}

struct history *history_new(void);
int history_push(struct history *hist, const char *);
// clear history buffer and free up strings stored in it
int history_clear(struct history *hist);
void history_show(struct history *hist, int n);

#endif
