#ifndef MANDELBROT_AVX_H
#define MANDELBROT_AVX_H

#include <immintrin.h>

#ifdef __x86_64__
#include <cpuid.h>

static inline int is_avx_supported(void)
{
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    return ecx & bit_AVX ? 1 : 0;
}
#endif // __x86_64__

__m256d map_avx(__m256d vec, double factor, double out_min);

void mandelbrot_avx(__m256d cx[4], __m256d cy[4], int max_iter, __m256d* iters);

#endif // MANDELBROT_AVX_H