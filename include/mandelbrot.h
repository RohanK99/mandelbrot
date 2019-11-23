#ifndef MANDELBROT_H
#define MANDELBROT_H

float map_float(float val, float in_min, float in_max, float out_min, float out_max);

int mandelbrot(float cx, float cy, int max_iter);

#endif // MANDELBROT_H