#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX_ENTRY 99999
#define HISTORY_NUM_WIDTH 5
struct history {
	char *buf[HISTORY_MAX_ENTRY];
	int cnt;
};

struct history *history_new(void);
int history_push(struct history *hist, const char *s);
// clear history buffer and free up strings stored in it
void history_clear(struct history *hist);
void history_show(struct history *hist, int n);

#endif
