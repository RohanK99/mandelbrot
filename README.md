# MANDELBROT

Mandelbrot visualizer built in C with SDL2 utilizing SIMD (AVX, SSE) for speed optimization. Optimization using vectorization, threading, loop unrolling and latency minimization. Colouring done with Bernstein polynomials.

## Build Steps

### Release Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Debug Build
```bash
mkdir build-debug && cd build-debug
cmake ..
make -j$(nproc)
```

## Running The Program
Usage: `./mandelbrot`
* `-i` : [A, S], A for AVX, S for SSE, and blank for reference

### Keyboards Shortcuts
Key q = Quit program
Key s = Screenshot image
