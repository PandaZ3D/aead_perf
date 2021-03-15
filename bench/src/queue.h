#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>
#include <sys/types.h>

// generic item sized queue
struct node {
  void* job;
  struct node* next;
};

struct queue {
  size_t q_size; // queue size
  size_t n_size; // node item size
  struct node* head;
  struct node* tail;
  int (*enqueue) (struct queue*, void*);
	void* (*dequeue) (struct queue*);
	bool (*empty) (struct queue*);
};

struct queue* new_queue(size_t);
void free_queue(struct queue*);

#endif /* __QUEUE_H__ */
