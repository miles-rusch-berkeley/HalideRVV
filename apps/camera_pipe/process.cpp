// #include "halide_benchmark.h"

#include "camera_pipe.h"
#ifndef NO_AUTO_SCHEDULE
#include "camera_pipe_auto_schedule.h"
#endif

#include "HalideBuffer.h"
// #include "halide_image_io.h"
// #include "halide_malloc_trace.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

static uint64_t read_cycles() {
    uint64_t cycles;
    asm volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
}

using namespace Halide::Runtime;
// using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc < 7) {
        //TODO: png file input/output
        printf("Usage: ./process input_dim color_temp gamma contrast sharpen timing_iterations\n"
               "e.g. ./process 33 3700 2.0 50 1.0 5 \n"); 
        return 0;
    }

#ifdef HL_MEMINFO
    halide_enable_malloc_trace();
#endif

    fprintf(stderr, "input dim: %s\n", argv[1]);
    // Buffer<uint16_t, 2> input = load_and_convert_image(argv[1]);
    int matrix_size = atoi(argv[1]);
    Buffer<uint16_t, 2> input(matrix_size, matrix_size);
    // Initialize gradient images
    for (int iy = 0; iy < matrix_size; iy++) {
        for (int ix = 0; ix < matrix_size; ix++) {
            input(ix, iy) = static_cast<uint16_t>((ix + iy) % 256);
        }
    }
    fprintf(stderr, "       %d %d\n", input.width(), input.height());
    Buffer<uint8_t, 3> output(((input.width() - 32) / 32) * 32, ((input.height() - 24) / 32) * 32, 3);

// #ifdef HL_MEMINFO
//     info(input, "input");
//     stats(input, "input");
//     // dump(input, "input");
// #endif

    // These color matrices are for the sensor in the Nokia N900 and are
    // taken from the FCam source.
    float _matrix_3200[][4] = {{1.6697f, -0.2693f, -0.4004f, -42.4346f},
                               {-0.3576f, 1.0615f, 1.5949f, -37.1158f},
                               {-0.2175f, -1.8751f, 6.9640f, -26.6970f}};

    float _matrix_7000[][4] = {{2.2997f, -0.4478f, 0.1706f, -39.0923f},
                               {-0.3826f, 1.5906f, -0.2080f, -25.4311f},
                               {-0.0888f, -0.7344f, 2.2832f, -20.0826f}};
    Buffer<float, 2> matrix_3200(4, 3), matrix_7000(4, 3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            matrix_3200(j, i) = _matrix_3200[i][j];
            matrix_7000(j, i) = _matrix_7000[i][j];
        }
    }

    float color_temp = (float)atof(argv[2]);
    float gamma = (float)atof(argv[3]);
    float contrast = (float)atof(argv[4]);
    float sharpen = (float)atof(argv[5]);
    // int timing_iterations = atoi(argv[6]);
    int blackLevel = 25;
    int whiteLevel = 1023;

    // check performance
    uint64_t n0,nf;
    printf("reading cycles\n");
    n0 = read_cycles();
    camera_pipe(input, matrix_3200, matrix_7000,
                color_temp, gamma, contrast, sharpen, blackLevel, whiteLevel,
                output);
    nf = read_cycles();
    printf("manual halide cycles=%lu,\n",nf-n0);

#ifndef NO_AUTO_SCHEDULE
    printf("reading cycles\n");
    n0 = read_cycles();
    camera_pipe_auto_schedule(input, matrix_3200, matrix_7000,
                            color_temp, gamma, contrast, 
                            sharpen, blackLevel, whiteLevel,
                            output);
    nf = read_cycles();
    printf("auto halide cycles=%lu,\n",nf-n0);
#endif

    // printf("input\n");
    // for (int iy = 0; iy < matrix_size; iy++) {
	//     for (int ix = 0; ix < matrix_size; ix++) {
	// 	     	printf("%d,",input(ix,iy));
	// 		    }
	//         printf("\n");
    // }
    // printf("OUTPUT IMAGE:\n");
    // for (int z = 0; z < 3; z++) {
    //     for (int iy = 0; iy < matrix_size; iy++) {
    //         for (int ix = 0; ix < matrix_size; ix++) {
    //             printf("%d,",output(ix,iy,z));
    //         }
    //         printf("\n");
    //     }
    // }
    fprintf(stderr, "        %d %d\n", output.width(), output.height());

    printf("Success!\n");
    return 0;
}
