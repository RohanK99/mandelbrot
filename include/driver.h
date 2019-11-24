#ifndef DRIVER_H
#define DRIVER_H

#include "utils.h"

extern int height;
extern int width;
extern int max_iter;
extern int avx_iter_factor;
extern double x_factor;
extern double y_factor;

void mandelbrot_driver(Bounds_t* bounds, pixel_t* pixels, int i_set);

#endif // DRIVER_H