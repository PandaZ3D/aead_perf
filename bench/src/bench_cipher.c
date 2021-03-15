// standard libraries here
#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

// include libraries to benchmark here
#include "bench_timer.h"
#include "mailbox.h"

// cryptographic ciphers
#include "chacha20.h"


#define SAMPLE_SIZE 9604
#define MAX_FILE_SIZE 64000

// bench name measurement, time (cycles or ns), size (bytes)
enum cipher_code {
  CHACHA20_BASE=1,
  CHACHA20_128=2,
  CHACHA20_256=3,
  CHACHA20_512=4,
  AES_CTR_BASE=5,
  AES_AESNI=6,
  CHACHA20_POLY_BASE=7, // include seperate vectorized chacha and poly??
  CHACHA20_POLY_128=8,
  CHACHA20_POLY_256=9,
  CHACHA20_POLY_512=10,
  AES_GCM_BASE=11,
  AES_GCM_AESNI=12 // further throughput??
};

enum measure_type {
  CYCLES=1,
  NANOSEC=2
};

static char* cipher_name[] = {
  "chacha_base",
  "chacha_128",
  "chacha_256",
  "aes_ctr_base",
  "aes_aesni",
  "chacha_poly_base",
  "chacha_poly_128",
  "chacha_poly_256",
  "chacha_poly_512",
  "aes_gcm_base",
  "aes_gcm_aesni"
};

static inline void write_all(int fd, char* data, int len) {
  int ret, to_write = len, written=0;
  do {
    ret = write(fd, data, to_write);
    if(ret < 0) {
      perror("died while writing");
      break;
    }
    written+= ret;
    to_write-= ret;
  } while(written < len);
}

static void inline read_all(int fd, char* data, int len) {
  int ret, to_read = len, readin=0;
  do {
    ret = read(fd, data + readin, to_read);
    if(ret < 0) {
      perror("died while reading");
      break;
    }
    readin+= ret;
    to_read-= ret;
  } while(readin < len);
}

// thread arguments here
struct benchmark_thread {
  pthread_t tid;
  struct mail_box* inbox;
  struct mail_box* outbox;
};

struct bench_work {
  uint8_t cipher : 4;
  uint8_t type : 4;
};

struct bench_result {
  uint64_t result;
  int file_size;
};

void get_data_file(char* data, int len) {
  char fname[64];
  snprintf(fname, 64, "inputs/input_%d.dat", len);
  int fd = open(fname, O_RDONLY);
  read_all(fd, data, len);
  if(close(fd) < 0) perror("cannot close file");
}


#define RESULTS_HEADER "name,size,time\n"
// static char* csv_hdr = {RESULTS_HEADER};
int create_csv(enum cipher_code C, enum measure_type M) {
  char fname[64];
  switch(M) {
    case CYCLES:
      snprintf(fname,64,"bench/results/data/%s_cycles.csv", cipher_name[C]);
      break;
    case NANOSEC:
      snprintf(fname,64,"bench/results/data/%s_nsec.csv", cipher_name[C]);
      break;
    default:
      break;
  }
  int fd = open(fname, O_WRONLY | O_CREAT, 0644);
  if(fd < 0) {
    perror("creating csv file");
  }
  dprintf(fd, "%s", RESULTS_HEADER);
  // // write header to csv file
  // write_all(csv_fd, csv_hdr, 16);
  return fd;
}

/*
void aes_ctr_encryption(const uint8_t plain_text[], uint8_t cipher_text[], 
    const uint32_t key_schedule[], //const uint32_t nonce, const uint64_t iv, 
    uint8_t counter[], int pt_length);

input, output, key, ptlen, (counter/nonce)

void chacha20_encryption_128 (
    const uint8_t in[], uint8_t out[],
    const uint8_t k[], const uint8_t n[], const uint32_t c, int inlen);

*/

struct chacha20_args {
  const uint8_t* in;
  uint8_t* out;
  const uint8_t* k;
  const uint8_t* n;
  const uint32_t c;
  int inlen;
};

void bench_chacha_base(void* args) {
  struct chacha20_args* ctx = (struct chacha20_args*) args;
  chacha20_encryption(
    ctx->in,
    ctx->out,
    ctx->k,
    ctx->n,
    ctx->c,
    ctx->inlen
  );
}

// thread work function to benchmark a single cipher
// for a given metric
void* benchmark_cipher(void* args) {
  struct benchmark_thread* bt;
  bt = (struct benchmark_thread*) args;
  // file sizes to be tested
  int fsize = 0; // from 0 to 64k in 20 byte increments
  // setup (thread affinity)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset); // set it to CPU0
  int status = pthread_setaffinity_np(bt->tid, sizeof(cpu_set_t), &cpuset);
  if(status != 0) {
    perror("could not set affinity!");
  }
  // cipher specific buffers
  uint8_t file_buffer[64000];
  uint8_t out_buffer[64000];
  uint8_t nonce[] = {
    0x00, 0x00, 0x09, 0x00, 
    0x27, 0x20, 0xfd, 0x63,
    0x6f, 0x75, 0xde, 0x14, 
    0x00, 0x00, 0x00, 0x01
  };
  uint32_t ctr = 1;
  // benchmark related vars
  struct bench_work* work;
  uint64_t (*get_end_time)(void);
  uint64_t (*get_start_time)(void);
  void (*cipher) (void* args);
  void* cipher_args;
  // main loop that waits for ciphers to benchmark
  struct bench_result res;
  while(1) {
    // recv work from main tread
    work = bt->inbox->recv(bt->inbox);
    // identify type of measurement
    switch(work->type) {
      case CYCLES:
        get_start_time = bench_readtsc_start;
        get_end_time = bench_readtsc_end;
        bench_tsc_init();
        break;
      case NANOSEC:
        get_start_time = stm_now;
        get_end_time = stm_now;
        stm_setup();
        break;
      default:
        break;
    };
    // determine cipher function to execute

    // for each file size
    // res.file_size = fsize;
    // read in file
    get_data_file(file_buffer, fsize);
    // measure a number of samples
    for(int i = 0; i < SAMPLE_SIZE; i++) {
      res.result = measure_time(100, 64, cipher, 
      cipher_args, get_start_time, get_end_time); // do i need to skip 64 on every measurement???
      // return result
      status = bt->outbox->send(bt->outbox, &res);
      if(status < 0) {
        perror("failed to send work to thread"); 
      }
    }
  }
}

void collect_samples(enum cipher_code N, 
  enum measure_type M,
  struct mail_box * workq,
  struct mail_box * results) {
  // create csv file
  int csv_fd = create_csv(N, M);
  // create work for thread
  struct bench_work work;
  work.cipher = N;
  work.type = M;
  // send work
  int status = workq->send(workq, &work);
  if(status < 0) {
    perror("failed to send work to thread"); 
  }
  // collect all samples
  struct bench_result* sample;
  int samples_collected = 0;
  do {
    sample = results->recv(results);
    if(sample == NULL) {
      fprintf(stderr, "sample queue return NULL\n");
      break;
    }
    samples_collected++;
    // write sample to csv file
    status = dprintf(csv_fd, "%s/%d,%d,%lu\n",
      cipher_name[N], sample->file_size, sample->file_size, sample->result);
  } while(samples_collected < SAMPLE_SIZE);
}

int main() {
  // ciphers to be tested
  
  // create queues for resutls and work
  struct mail_box results;
  init_mail_box(&results, sizeof(struct bench_result));
  struct mail_box workq;
  init_mail_box(&workq, sizeof(struct bench_work));

  // initalize worker thread
  struct benchmark_thread worker;
  worker.inbox = &workq;
  worker.outbox = &results;
  int ret = pthread_create(&worker.tid, NULL, benchmark_cipher, &worker);
  if(ret < 0) {
    perror("thread creation");
    return -1;
  }

  // for each cipher
  for(int i = CHACHA20_BASE; i <= CHACHA20_BASE; i++) {
    // collect both cycles and timing
    collect_samples(i, CYCLES, &workq, &results);
  }
}