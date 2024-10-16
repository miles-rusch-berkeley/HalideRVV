#include "sconv_rvv.h"
#include "HalideBuffer.h"
// #include "HalideRuntime.h"           
#include <stdio.h>
#include <stdint.h>

static uint64_t read_cycles() {
    // Enable user/supervisor use of perf counters
    // if (supports_extension('S'))
    //     printf("supports S\n");
    //     write_csr(scounteren, -1);
    // if (supports_extension('U'))
    //     printf("supports U\n");
    //     write_csr(mcounteren, -1);
        
    uint64_t cycles;
    asm volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
    }

int main(int argc, char **argv) {

    
    int matrix_size = atoi(argv[1]);
    printf("matrix_size=%d\n",matrix_size);

    // Let's make a buffer for our input and output.
    Halide::Runtime::Buffer<float> mat_A(matrix_size, matrix_size);
    Halide::Runtime::Buffer<float> kx(3);
    // Halide::Runtime::Buffer<float> ky(3);
    Halide::Runtime::Buffer<float> output_halide(matrix_size, matrix_size);
    Halide::Runtime::Buffer<float> output_ref(matrix_size, matrix_size);

    for (int iy = 0; iy < matrix_size; iy++) {
        for (int ix = 0; ix < matrix_size; ix++) {
            mat_A(ix, iy) = (float) ((ix * iy) % 256);
    //        printf("mat_A(%d, %d)=%d\n",
    //        ix, iy, mat_A(ix, iy));
        }
    }
    for (int i = 0; i < 3; i++) {
        kx(i) = 1.0f/3.0f;
        // ky(i) = 1.0f/3.0f;
    }
// 0.25203252f*inb(x - 1, y) + 0.49593496f*inb(x, y) + 0.25203252f*inb(x + 1, y);
//        	rows(x, y) = 0.5483871f*cols(x, y - 1) + 0.01075269f*cols(x, y) + 0.44086022f*
    // check results
    uint64_t n0,nf;
    printf("reading cycles\n");
    n0 = read_cycles();
    int error = sconv_rvv(mat_A, kx, output_halide);
    nf = read_cycles();
    printf("halide cycles=%lu,\n",nf-n0);

    if (error) {
        printf("Halide returned an error: %d\n", error);
        return -1;
    }

    for (int iy = 0; iy < matrix_size; iy++) {
        for (int ix = 0; ix < matrix_size; ix++) {
            printf("%.4g,",output_halide(ix,iy));
        }
        printf("output_test\n");
    }

    return 0;
}
