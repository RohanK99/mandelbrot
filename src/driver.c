#include <time.h>
#include <omp.h>

#include "driver.h"
#include "mandelbrot.h"
#include "mandelbrot_avx.h"
#include "mandelbrot_sse.h"

void mandelbrot_driver(Bounds_t* bounds, pixel_t* pixels, int i_set) {
    __m256d _3210 = _mm256_set_pd(3.0, 2.0, 1.0, 0.0);
    __m256d _4    = _mm256_set1_pd(4.0);

    time_t start = clock();
    #ifdef __x86_64__
    if ((i_set == avx) && is_avx_supported()) {
        #pragma openmp parallel for schedule(guided)
        for (int y = 0; y < height; ++y) {
            __m256d vy = _mm256_set1_pd((double)y);
            __m256d cvy = map_avx(vy, y_factor, bounds->min_y);
            __m256d cy[4] = {cvy, cvy, cvy, cvy};
            for (int x = 0; x < width; x += 16) {
                __m256d vx0 = _mm256_add_pd(_3210, _mm256_set1_pd((double)x));
                __m256d vx1 = _mm256_add_pd(_4, vx0);
                __m256d vx2 = _mm256_add_pd(_4, vx1);
                __m256d vx3 = _mm256_add_pd(_4, vx2);
                __m256d cx[4] = {map_avx(vx0, x_factor, bounds->min_x), map_avx(vx1, x_factor, bounds->min_x),
                                    map_avx(vx2, x_factor, bounds->min_x), map_avx(vx3, x_factor, bounds->min_x)};

                union _256d_4 res = {_mm256_setzero_pd(), _mm256_setzero_pd(), _mm256_setzero_pd(), _mm256_setzero_pd()};
                mandelbrot_avx(cx, cy, max_iter, res.v);
                int offset = 0;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        color_poly(res.a[i][j], max_iter, pixels, width*y + (x+offset++));
                    }
                }
            }
        }
    } else if (i_set == sse) {
        #pragma openmp parallel for schedule(guided)
        for (int y = 0; y < height; ++y) {
            float fy = (float)y;
            __m128 vy = _mm_set_ps1(fy);
            __m128 cy = map_sse(vy, y_factor, bounds->min_y);
            for (int x = 0; x < width; x += 4){
                float fx = (float)x;
                __m128 vx = _mm_add_ps(_mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f), _mm_set_ps1(fx));
                __m128 cx = map_sse(vx, x_factor, bounds->min_x);

                union _128i iters;
                iters.v = mandelbrot_sse(cx, cy, max_iter);
                for (int i = 0; i < 4; i++) {
                    color_poly(iters.a[i], max_iter, pixels, width*y + (x+i));
                }
            }
        }
    }
    #endif // __x86_64__
    else {
        #pragma openmp parallel for schedule(guided)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float cx = map_float(x, 0, width, bounds->min_x, bounds->max_x);
                float cy = map_float(y, 0, height, bounds->min_y, bounds->max_y);
                int iter = mandelbrot(cx, cy, max_iter);
                color_poly(iter, max_iter, pixels, width*y + x);
            }
        }
    }

    time_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%f\n", time_spent);
}