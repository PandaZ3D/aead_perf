// standard libraries here
// #define _GNU_SOURCE
#include <stdio.h>
// #include <sched.h>
// #include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

// include libraries to benchmark here
#include "bench_timer.h"
#include "bench_const.h"

// cryptographic ciphers
#include "chacha20.h"

struct chacha20_args {
  const uint8_t* in;
  uint8_t* out;
  const uint8_t* k;
  const uint8_t* n;
  uint32_t c;
  int inlen;
};

void collect_samples(enum cipher_code N, 
  enum measure_type M, void* cipher_args,
  void* cipher_func);

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

int main() {
  uint8_t file_buffer[64000];
  uint8_t out_buffer[64000];

  struct chacha20_args arx_args;
  arx_args.in = file_buffer;
  arx_args.out = out_buffer;
  arx_args.k = key;
  arx_args.n = nonce;
  arx_args.c = 1;
  // all cipher functions
  void* cipher_func[] = {bench_chacha_base};
  // all cipher arguments
  void* cipher_args[] = {&arx_args};


  // for each cipher
  // for(int i = CHACHA20_BASE; i <= CHACHA20_BASE; i++) {
    // collect both cycles and timing
  collect_samples(CHACHA20_BASE, CYCLES, 
    cipher_args[CHACHA20_BASE], cipher_func[CHACHA20_BASE]);
  // } 
  // uint64_t res = measure_time(100, 64, bench_chacha_base, 
  //     cipher_args[CHACHA20_BASE], bench_readtsc_start, bench_readtsc_end);
  // printf("res: %lu\n", res);
  return 0;
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

void set_file_size(
  enum cipher_code C, void* args, int ptlen
  ) {
  switch(C) {
    case CHACHA20_BASE:
      ((struct chacha20_args*) args)->inlen = ptlen;
      break;
    default:
      fprintf(stderr, "how did i get here??\n");
      break;
  };
  // (void) args;
}

void collect_samples(
  enum cipher_code N, // the cipher we are using 
  enum measure_type M, // the metric we are measuring
  void* cipher_args,
  void* cipher_func) {
  // create csv file
  int csv_fd = create_csv(N, M);
  // configuration based on specific cipher
  int file_size = 1;
  // collect all samples (for a single file size)
  int samples_collected = 0;
  uint64_t result;
  set_file_size(N, cipher_args, file_size);
  // warm up the cache
  measure_time(100, 0,
      cipher_func, cipher_args, bench_readtsc_start, bench_readtsc_end);
  do {
    result = measure_time(100, 0,
      cipher_func, cipher_args, bench_readtsc_start, bench_readtsc_end);
    samples_collected++;
    // write sample to csv file
    dprintf(csv_fd, "%s/%d,%d,%lu\n",
      cipher_name[N], file_size, file_size, result);
  } while(samples_collected < SAMPLE_SIZE);

  // some final sizes
  
  close(csv_fd);
}