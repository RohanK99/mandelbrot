#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <SDL2/SDL.h>
#include <omp.h>
#include <time.h>

#include "mandelbrot.h"
#include "mandelbrot_sse.h"
#include "mandelbrot_avx.h"

int width = 800;
int height = 800;
int max_iter = 50;
double x_factor;
double y_factor;
int prev_mouse_x;
int prev_mouse_y;

typedef struct Bounds {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
} Bounds_t;

static Bounds_t bounds = { -2, 2, -2, 2 };
typedef double coord_t;

typedef enum ZoomDir {
    ZOOM_IN = 0,
    ZOOM_OUT
} ZoomDir_t;

typedef enum instruction_set {
    def,
    avx,
    sse
} instruction_set_t;

union _128i {
    __m128i v;
    int a[4];
};

union _256d_4 {
    __m256d v[4];
    double  a[4][4];
};

void color_poly(int n, int iter_max, int* colors) {
	// map n on the 0..1 interval
	double t = (double)n/(double)iter_max;

	// Use smooth polynomials for r, g, b
	colors[0] = (int)(9*(1-t)*t*t*t*255);
	colors[1] = (int)(15*(1-t)*(1-t)*t*t*255);
	colors[2] =  (int)(8.5*(1-t)*(1-t)*(1-t)*t*255);
}

void color(int n, int iter_max, int* colors) {
    int N = 256; // colors per element
    int N3 = N * N * N;
    // map n on the 0..1 interval (real numbers)
    double t = (double)n/(double)iter_max;
    // expand n on the 0 .. 256^3 interval (integers)
    n = (int)(t * (double) N3);

    colors[2] = n/(N * N);
    int nn = n - colors[2] * N * N;
    colors[0] = nn/N;
    colors[1] = nn - colors[0] * N;
}

#ifdef __x86_64__
#include <cpuid.h>

static inline int
is_avx_supported(void)
{
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    return ecx & bit_AVX ? 1 : 0;
}
#endif // __x86_64__

void mandelbrot_driver(Bounds_t *bounds, int* color_arr, int i_set) {
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
                        color_arr[width*y + (x+offset++)] = res.a[i][j];
                    }
                }
            }
        }
    } else if (i_set == sse) {
        #pragma openmp for collapse(2)
        for (int iy = 0; iy < height; ++iy) {
            float fy = (float)iy;
            __m128 vy = _mm_set_ps1(fy);
            __m128 cy = map_sse(vy, y_factor, bounds->min_y);
            for (int ix = 0; ix < width; ix += 4){
                float fx = (float)ix;
                __m128 vx = _mm_add_ps(_mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f), _mm_set_ps1(fx));
                __m128 cx = map_sse(vx, x_factor, bounds->min_x);

                union _128i pixels;
                pixels.v = mandelbrot_sse(cx, cy, max_iter);
                for (int i = 0; i < 4; i++) {
                    color_arr[width*iy + (ix+i)] = pixels.a[i];
                }
            }
        }
    }
    #endif // __x86_64__
    else {
        #pragma openmp for collapse(2)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float cx = map(x, 0, width, bounds->min_x, bounds->max_x);
                float cy = map(y, 0, height, bounds->min_y, bounds->max_y);
                int iter = mandelbrot(cx, cy, max_iter);
                color_arr[width*y + x] = iter;
            }
        }
    }

    time_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("%f\n", time_spent);
}

void update_display_cfg(Bounds_t *bounds) {
    x_factor = (bounds->max_x - bounds->min_x) / width;
    y_factor = (bounds->max_y - bounds->min_y) / height;
}

double map_double(double val, double in_max, double out_min, double out_max) {
    return (val) * (out_max - out_min) / (in_max) + out_min;
}

void zoom(coord_t mouse_x, coord_t mouse_y, ZoomDir_t zoom_dir) {
    const double zoom_factor = (zoom_dir == ZOOM_IN) ? 0.8 : 1.2;
    coord_t min_x = mouse_x + zoom_factor * (bounds.min_x - mouse_x);
    coord_t max_x = mouse_x + zoom_factor * (bounds.max_x - mouse_x);
    coord_t min_y = mouse_y + zoom_factor * (bounds.min_y - mouse_y);
    coord_t max_y = mouse_y + zoom_factor * (bounds.max_y - mouse_y);

    bounds.min_x = min_x;
    bounds.max_x = max_x;
    bounds.min_y = min_y;
    bounds.max_y = max_y;
    
    update_display_cfg(&bounds);
}

int main(int argc, char** argv) {

    // parse options
    instruction_set_t i_set;
    int opt;
    while((opt = getopt(argc, argv, "i:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':
            switch(*optarg) {
                case 'A':
                    i_set = avx;
                    break;
                case 'S':
                    i_set = sse;
                    break;
                default:
                    i_set = def;
                    break;
            }
            break;
        }
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Event     event;

    int* color_arr = malloc(sizeof(int)*width*height);

    window = SDL_CreateWindow("mandelbrot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, width, height);

    update_display_cfg(&bounds);
    mandelbrot_driver(&bounds, color_arr, i_set);

    while(1) {
        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == 'q'))
            {
                break;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);

                double delta_x = map_double(prev_mouse_x - x, width, bounds.min_x, bounds.max_y);
                double delta_y = map_double(prev_mouse_y - y, height, bounds.min_y, bounds.max_y);

                double factor_x = (delta_x > 0) ? 0.05 : -0.05;
                double factor_y = (delta_y > 0) ? 0.05 : -0.05;
                bounds.min_x += delta_x * factor_x;
                bounds.max_x += delta_x * factor_x;
                bounds.min_y += delta_y * factor_y;
                bounds.max_y += delta_y * factor_y;

                update_display_cfg(&bounds);
                mandelbrot_driver(&bounds, color_arr, i_set);

                prev_mouse_x = x;
                prev_mouse_y = y;
            }
            else if (event.type == SDL_WINDOWEVENT_RESIZED)
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                width = (double)w;
                height = (double)h;
            }
            else if(event.type == SDL_MOUSEWHEEL)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                coord_t mouse_x = map_double(x, width, bounds.min_x, bounds.max_x);
                coord_t mouse_y = map_double(y, height, bounds.min_y, bounds.max_y);
                if(event.wheel.y > 0)
                {
                    zoom(mouse_x, mouse_y, ZOOM_IN);
                }
                else if(event.wheel.y < 0)
                {
                    zoom(mouse_x, mouse_y, ZOOM_OUT);
                }
                mandelbrot_driver(&bounds, color_arr, i_set);
            }
        }

        // DRAWING
        for (int h = 0; h < height; h++) {
            for (int w = 0; w < width; w++) {
                int colors[3];
                color_poly(color_arr[w + (h*width)], max_iter, (void*)&colors);
                SDL_SetRenderDrawColor(renderer, colors[0], colors[1], colors[2], 255);
                SDL_RenderDrawPoint(renderer, w, h);
            }
        }
    }
    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}
