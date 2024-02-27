#ifndef LIST_H
#define LIST_H

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *head)
{
	head->next = head;
	head->prev = head;
}

static inline void list_add_tail(struct list_head *node, struct list_head *head) {
	
	struct list_head *tail = head->prev;

	tail->next = node;
	node->prev = tail;
	node->next = head;
	head->prev = node;

}

static inline int list_empty(const struct list_head *head) {
	return (head->next == head);
}

#define list_for_each(node, head) \
	for (node = (head)->next; node != (head); node = node->next)

#endif
