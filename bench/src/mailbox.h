#ifndef __MAIL_BOX_H__
#define __MAIL_BOX_H__

#include <pthread.h>
#include "queue.h"

// synchonous mailbox (queue)
struct mail_box {
  struct queue* q;
  pthread_mutex_t lock;
  pthread_cond_t cv;
  int (*send) (struct mail_box*, void*);
  void* (*recv) (struct mail_box*);
};

// send message of size `msgsize', returns success or failure
static inline int send_msg(struct mail_box* mbox, void* msg) {
  pthread_mutex_lock(&mbox->lock);
    int status = mbox->q->enqueue(mbox->q, msg);
    if(status == 0)
      pthread_cond_signal(&mbox->cv);
  pthread_mutex_unlock(&mbox->lock);
  return status;
}

// returns message from mailbox (must be free'd), can be NULL
static inline void* recv_msg(struct mail_box* mbox) {
  pthread_mutex_lock(&mbox->lock);
    while(mbox->q->empty(mbox->q))
      pthread_cond_wait(&mbox->cv, &mbox->lock);
    void* msg = mbox->q->dequeue(mbox->q);
  pthread_mutex_unlock(&mbox->lock);
  return msg;
}

static inline int init_mail_box(struct mail_box* mbox, size_t msgsize) {
  if((mbox->q = new_queue(msgsize))) {
    mbox->lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    mbox->cv = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    mbox->send = send_msg;
    mbox->recv = recv_msg;
    return 0;
  }
  return -1;
}

static inline void destroy_mail_box(struct mail_box* mbox) {
  if(mbox) {
    free_queue(mbox->q);
  }
}

#endif /* __MAIL_BOX_H__ */
