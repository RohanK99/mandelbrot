# MANDELBROT

Mandelbrot visualizer built in C with SDL2 utilizing SIMD (AVX, SSE) for speed optimization. Optimization using vectorization, threading, loop unrolling and latency minimization. Colouring done with Bernstein polynomials.

## Build Dependencies
```
libsdl2-dev
libomp-dev
```

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
cmake -DCMAKE_BUILD_TYPE=debug ..
make -j$(nproc)
```

## Running The Program
Usage: `./mandelbrot`
* `-i`: [A, S, R], A for AVX, S for SSE, and R for reference
* if `-i` is not passed then reference generator is used.

### Keyboards Shortcuts
Key q = Quit program\
Key s = Screenshot image
