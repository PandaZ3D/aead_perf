#include <stdlib.h>
#include <string.h>

#include "queue.h"

int enqueue(struct queue* q, void* job);
void* dequeue(struct queue* q);
bool is_empty(struct queue* q);

struct node* new_node(size_t memsize, void* job) {
  struct node* n = (struct node*) malloc(sizeof(struct node));
  if(n) {
    n->job = malloc(memsize);
    if(n->job == NULL) {
      free(n);
      return NULL;
    }
    n->next = NULL;
    memcpy(n->job, job, memsize);
  }
  return n;
}

void free_node(struct node* n) {
  if(n) {
    if(n->job) {
      free(n->job);
    free(n);
    }
  }
}

void free_nodes(struct queue* q) {
  struct node* n = q->head;
  do {
    q->head = n->next;
    free_node(n);
  } while(q->head != NULL);
}

struct queue* new_queue(size_t memsize) {
  struct queue* q = (struct queue*) malloc(sizeof(struct queue));
  if(q) {
    q->head = q->tail = NULL;
    q->n_size = memsize;
    q->q_size = 0;
    q-> enqueue = enqueue;
    q->dequeue = dequeue;
    q->empty = is_empty;
  }
  return q;
}

void free_queue(struct queue* q) {
  if(q) {
    if(q->q_size > 0)
      free_nodes(q);
    free(q);
  }
}

// enqueue job of q_size
int enqueue(struct queue* q, void* job) {
  struct node* n = new_node(q->n_size, job);
  if(n) {
    if(q->q_size == 0)
      q->head = n;
    else
      q->tail->next = n;
    q->tail = n;
    q->q_size++;
    return 0;
  }
  return -1;
}

// returns job object (must be free'd) or NULL
void* dequeue(struct queue* q) {
  struct node* n = q->head;
  if(n) {
    if(q->q_size == 1)
      q->head = q->tail = NULL;
    else
      q->head = n->next;
    q->q_size--;
    void* job = n->job;
    free(n);
    return job;
  }
  return n;
}

// empty status
bool is_empty(struct queue* q) {
  return (q->q_size == 0);
}
