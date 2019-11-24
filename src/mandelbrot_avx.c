#include "mandelbrot_avx.h"

#include <stdint.h>

#define MANDEL_AVX_DEPENDENT(i) \
        y[i]  = _mm256_add_pd(_mm256_add_pd(xy[i],xy[i]),cy[i]); \
        x[i]  = _mm256_add_pd(_mm256_sub_pd(x2[i],y2[i]),cx[i]); \
        mag[i] = _mm256_add_pd(x2[i],y2[i]); \
        mask[i]  = _mm256_cmp_pd(mag[i], _4, _CMP_LT_OQ); \
        iters[i] = _mm256_add_pd(iters[i], _mm256_and_pd(mask[i], _1));

#define MANDEL_AVX_INDEPENDENT(i) \
        xy[i] = _mm256_mul_pd(x[i],y[i]); \
        x2[i] = _mm256_mul_pd(x[i],x[i]); \
        y2[i] = _mm256_mul_pd(y[i],y[i]);

#define MANDEL_AVX_ITERATION() \
        MANDEL_AVX_INDEPENDENT(0) \
        MANDEL_AVX_DEPENDENT(0) \
        MANDEL_AVX_INDEPENDENT(1) \
        MANDEL_AVX_DEPENDENT(1) \
        MANDEL_AVX_INDEPENDENT(2) \
        MANDEL_AVX_DEPENDENT(2) \
        MANDEL_AVX_INDEPENDENT(3) \
        MANDEL_AVX_DEPENDENT(3) \

#define MANDEL_AVX_MASK() \
        cmp_mask      =   \
            (_mm256_movemask_pd (mask[0]) << 4 ) \
          | (_mm256_movemask_pd (mask[1])      ) \
          | (_mm256_movemask_pd (mask[2]) << 12) \
          | (_mm256_movemask_pd (mask[3]) << 8);

__m256d map_avx(__m256d vec, double factor, double out_min) {
    __m256d mul_vec = _mm256_mul_pd(vec, _mm256_set1_pd(factor));
    __m256d res     = _mm256_add_pd(mul_vec, _mm256_set1_pd(out_min));
    return res;
}

void mandelbrot_avx(__m256d cx[4], __m256d cy[4], int iter_factor, __m256d* iters) {
    __m256d _4 = _mm256_set1_pd(4.0);
    __m256d _1 = _mm256_set1_pd(1.0);
    __m256d _0 = _mm256_set1_pd(0.0);

    __m256d x[4] = {cx[0], cx[1], cx[2], cx[3]};
    __m256d y[4] = {cy[0], cy[1], cy[2], cy[3]};

    __m256d x2[4], y2[4], mask[4], xy[4];
    __m256d mag[4];

    uint32_t cmp_mask = 0;
    
    for (int iter = 6 + iter_factor; iter > 0; --iter) {
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()
        MANDEL_AVX_ITERATION()

        MANDEL_AVX_MASK()

        if (!cmp_mask) {
            return;
        }
    }

    MANDEL_AVX_ITERATION()
    MANDEL_AVX_ITERATION()
}