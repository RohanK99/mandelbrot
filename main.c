#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "driver.h"

int prev_mouse_x;
int prev_mouse_y;

static Bounds_t bounds = { -2, 2, -2, 2 };

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
    SDL_Texture*  texture;
    SDL_Surface*  surface;

    char     filename[64];
    pixel_t* pixels = malloc(sizeof(uint32_t)*width*height);

    window   = SDL_CreateWindow("mandelbrot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STATIC, width, height);
    surface  = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_BGRA8888);

    SDL_RenderSetLogicalSize(renderer, width, height);

    update_display_cfg(&bounds);
    mandelbrot_driver(&bounds, pixels, i_set);

    int quit = 0;
    int num_screenshots = 0;
    int x, y, w, h;
    while(!quit) {
        SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));

        SDL_WaitEvent(&event);

        switch(event.type) {
            case SDL_QUIT:
                quit = 1;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == 'q') {
                    quit = 1;
                }
                if (event.key.keysym.sym == 's') {
                    sprintf(filename, "screenshot(%d).bmp", num_screenshots++);
                    screenshot(renderer, surface, filename);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
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
                mandelbrot_driver(&bounds, pixels, i_set);
                
                prev_mouse_x = x;
                prev_mouse_y = y;

            case SDL_WINDOWEVENT_RESIZED:
                SDL_GetWindowSize(window, &w, &h);
                width = (double)w;
                height = (double)h;

            case SDL_MOUSEWHEEL:
                SDL_GetMouseState(&x, &y);
                coord_t mouse_x = map_double(x, width, bounds.min_x, bounds.max_x);
                coord_t mouse_y = map_double(y, height, bounds.min_y, bounds.max_y);
                if(event.wheel.y > 0)
                {
                    zoom(mouse_x, mouse_y, ZOOM_IN, &bounds);
                }
                else if(event.wheel.y < 0)
                {
                    zoom(mouse_x, mouse_y, ZOOM_OUT, &bounds);
                }
                mandelbrot_driver(&bounds, pixels, i_set);
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    // Close and destroy the window
    free(pixels);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}
