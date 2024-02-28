#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "history.h"

//struct history {
//	char *buf[HISTORY_MAX_ENTRY];
//	int cnt;
//}

struct history *hist;

int history_init(void) {

	if (hist != NULL) {
		fprintf(stderr, "error: history already init'd\n");
		return -1;
	}

	hist = (struct history *)malloc(sizeof(struct history));
	if (hist == NULL) {
		fprintf(stderr, "error: %s\n", "malloc() failed");
		return -1;
	}

	hist->buf[0] = NULL;
	hist->cnt = 0;

	return 0;
}

int history_push(const char *cmd) {

	char *dup;

	if (hist == NULL) {
		fprintf(stderr, "error: history not init'd\n");
		return -1;
	}

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
void history_clear() {

	int i;

	if (hist == NULL) {
		fprintf(stderr, "error: history not init'd\n");
		return;
	}

	for (i = 0; i < hist->cnt; i++) {
		free(hist->buf[i]);
		hist->buf[i] = NULL;
	}
	hist->cnt = 0;

	return;
}

void history_show(long int n) {

	long int start;
	long int i;

	if (hist == NULL) {
		fprintf(stderr, "error: history not init'd\n");
		return;
	}
	
	// XXX: use max() macro?
	n = (n > 10)? 10 : n;
	start = ((hist->cnt - n) > 0)? hist->cnt - n : 0;

	for (i = start; i < hist->cnt; i++) {
		printf("%*ld  %s\n", HISTORY_NUM_WIDTH, i, hist->buf[i]);
	}

	return;
}

void history_free() {

	if (hist == NULL) {
		fprintf(stderr, "error: history not init'd\n");
		return;
	}

	// XXX: error handling
	free(hist);
}
