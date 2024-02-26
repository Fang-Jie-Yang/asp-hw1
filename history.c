#include <stdlib.h>
#include <stdio.h>

#include "history.h"

//struct history {
//	char *buf[HISTORY_MAX_ENTRY];
//	int cnt;
//}

struct history *history_new(void) {

	struct history *hist;

	hist = malloc(sizeof(history));
	if (hist == NULL) {
		fprintf(stderr, "error: %s\n", "malloc() failed");
		return NULL;
	}

	hist->buf[0] = NULL;
	hist->cnt = 0;

	return hist;
}

int history_push(struct history *hist, const char *s) {

	if (hist->cnt >= HISTORY_MAX_ENTRY) {
		fprintf(stderr, "history: exceeding %d commands\n", MAX_HISTORY_ENTRY);
		return -1;
	}

	hist->buf[hist->cnt] = s
	hist->cnt += 1;

	return 0;
}

// clear history buffer and free up strings stored in it
void history_clear(struct history *hist) {

	hist->buf[0] = NULL;
	hist->cnt = 0;

	return;
}

void history_show(struct history *hist, int n) {

	int start;
	
	// XXX: use max() macro?
	n = (n > 10)? 10 : n;
	start = ((hist->cnt - n) > 0)? hist->cnt - n : 0;

	for (int i = start; i < hist->cnt; i++) {
		printf("%*d  %s", HISTORY_NUM_WIDTH, i, hist->buf[i]);
	}

	return;
}
