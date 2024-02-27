#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "history.h"

//struct history {
//	char *buf[HISTORY_MAX_ENTRY];
//	int cnt;
//}

struct history *history_new(void) {

	struct history *hist;

	hist = (struct history *)malloc(sizeof(struct history));
	if (hist == NULL) {
		fprintf(stderr, "error: %s\n", "malloc() failed");
		return NULL;
	}

	hist->buf[0] = NULL;
	hist->cnt = 0;

	return hist;
}

int history_push(struct history *hist, const char *cmd) {

	char *dup;

	if (*cmd == '\0') {
		return 0;
	}

	// we don't push if cmd == the previous one
	if (hist->cnt > 0 && strcmp(cmd, hist->buf[hist->cnt - 1]) == 0) {
		return 0;
	}

	if (hist->cnt >= HISTORY_MAX_ENTRY) {
		fprintf(stderr, "history: exceeding %d commands\n", HISTORY_MAX_ENTRY);
		return -1;
	}

	dup = strdup(cmd);
	if (dup == NULL) {
		fprintf(stderr, "history: failed to record command\n");
		return -1;
	}
	hist->buf[hist->cnt] = dup;
	hist->cnt += 1;

	return 0;
}

// clear history buffer and free up strings stored in it
void history_clear(struct history *hist) {

	int i;

	for (i = 0; i < hist->cnt; i++) {
		free(hist->buf[i]);
		hist->buf[i] = NULL;
	}
	hist->cnt = 0;

	return;
}

void history_show(struct history *hist, int n) {

	int start;
	
	// XXX: use max() macro?
	n = (n > 10)? 10 : n;
	start = ((hist->cnt - n) > 0)? hist->cnt - n : 0;

	for (int i = start; i < hist->cnt; i++) {
		printf("%*d  %s\n", HISTORY_NUM_WIDTH, i, hist->buf[i]);
	}

	return;
}
