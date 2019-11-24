#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct Bounds {
    double min_x;
    double max_x;
    double min_y;
    double max_y;
} Bounds_t;

typedef double coord_t;

typedef enum ZoomDir {
    ZOOM_IN = 0,
    ZOOM_OUT
} ZoomDir_t;

typedef enum instruction_set {
    ref,
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

typedef union pixel {
    uint32_t bgra;
    uint8_t  a[4];
} pixel_t;

void color_poly(int n, int iter_max, pixel_t* pixel, int index);

void color(int n, int iter_max, int* colors);

void screenshot(SDL_Renderer* renderer, SDL_Surface* surface, char* filename);

double map_double(double val, double in_max, double out_min, double out_max);

void update_display_cfg(Bounds_t* bounds);

void zoom(coord_t mouse_x, coord_t mouse_y, ZoomDir_t zoom_dir, Bounds_t* bounds);

void updateIterations(ZoomDir_t zoom_dir, instruction_set_t i_set);

#endif // UTILS_H
