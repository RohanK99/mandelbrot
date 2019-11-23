#include "utils.h"

int height = 800;
int width = 800;
int max_iter = 50;
double x_factor;
double y_factor;

void color_poly(int n, int iter_max, pixel_t* pixel, int index) {
	// map n on the 0..1 interval
	double t = (double)n/(double)iter_max;

	// Use smooth polynomials for r, g, b
    pixel[index].a[0] = 255; // full alpha
	pixel[index].a[1] = (int)(9*(1-t)*t*t*t*255); // red
	pixel[index].a[2] = (int)(15*(1-t)*(1-t)*t*t*255); // green
	pixel[index].a[3] = (int)(8.5*(1-t)*(1-t)*(1-t)*t*255); // blue
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

double map_double(double val, double in_max, double out_min, double out_max) {
    return (val) * (out_max - out_min) / (in_max) + out_min;
}

void screenshot(SDL_Renderer* renderer, SDL_Surface* surface, char* filename) {
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_BGRA8888, surface->pixels, surface->pitch);
    SDL_SaveBMP(surface, filename);
}

void update_display_cfg(Bounds_t* bounds) {
    x_factor = (bounds->max_x - bounds->min_x) / width;
    y_factor = (bounds->max_y - bounds->min_y) / height;
}

void zoom(coord_t mouse_x, coord_t mouse_y, ZoomDir_t zoom_dir, Bounds_t* bounds) {
    const double zoom_factor = (zoom_dir == ZOOM_IN) ? 0.8 : 1.2;
    coord_t min_x = mouse_x + zoom_factor * (bounds->min_x - mouse_x);
    coord_t max_x = mouse_x + zoom_factor * (bounds->max_x - mouse_x);
    coord_t min_y = mouse_y + zoom_factor * (bounds->min_y - mouse_y);
    coord_t max_y = mouse_y + zoom_factor * (bounds->max_y - mouse_y);

    bounds->min_x = min_x;
    bounds->max_x = max_x;
    bounds->min_y = min_y;
    bounds->max_y = max_y;
    
    update_display_cfg(bounds);
}