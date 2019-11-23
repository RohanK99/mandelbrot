cmake_minimum_required(VERSION 3.9)

SET(PROJECT_NAME mandelbrot)

project(${PROJECT_NAME} LANGUAGES C)

SET(AVX_FLAG "-mavx")
add_definitions(${AVX_FLAG})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(OpenMP)

SET(SOURCES main.c
            include/driver.h
            include/utils.h
            include/mandelbrot.h
            include/mandelbrot_avx.h
            include/mandelbrot_sse.h
            src/driver.c
            src/utils.c
            src/mandelbrot.c
            src/mandelbrot_avx.c
            src/mandelbrot_sse.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_C)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)