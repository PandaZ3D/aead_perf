#ifndef __BENCH_TIMER_H__
#define __BENCH_TIMER_H__

#if defined(__x86_64__) || defined(__amd64__)
// #include <immintrin.h> // vector instructions
#include <x86intrin.h> // used for TSC
#else
#error Using intrinsics requires an x86 machine
#endif

#include <stdint.h>

/* time based on sokol time library */
#define SOKOL_IMPL
#include "sokol_time.h"

static inline uint64_t 
__attribute__((always_inline)) 
bench_readtsc_start() {
	// serialize and retire previous instructions
	_mm_lfence();
	// read actual TSC 
	return __rdtsc();
}

static inline uint64_t
__attribute__((always_inline))
bench_readtsc_end() {
	unsigned int aux; // read TSC_AUX[31:0] register
	// read actual TSC, retire previous instructions
	uint64_t tsc = __rdtscp(&aux);
	// serialize and retire previous instructions
	_mm_lfence();
	return tsc;
}

/* run readtsc 3 times to warm up cache */
void bench_tsc_init() {
	bench_readtsc_start();
	bench_readtsc_end();

	bench_readtsc_start();
	bench_readtsc_end();
	
	bench_readtsc_start();
	bench_readtsc_end();
}

uint64_t measure_time(
	int iterations,
	int ignore_iter, 
	void (*function)(void* args),
  void * args,
  uint64_t (*get_start_time)(void),
  uint64_t (*get_end_time)(void)
) {
	int i = 0;
	uint64_t start, end;
	for(i = 0; i < ignore_iter; i++) {
		function(args);
	}
	start = get_end_time();
	do {
		function(args);
	} while(++i < iterations);
	end = get_start_time();
	return (end - start) / iterations;
}

#endif