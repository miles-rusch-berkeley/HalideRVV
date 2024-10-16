#include <chrono>
#include <cstdio>

#include "lens_blur.h"
#include "lens_blur_auto_schedule.h"

#include "HalideBuffer.h"
#include "halide_benchmark.h"
// #include "halide_image_io.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc < 7) {
        printf("Usage: ./process input.png slices focus_depth blur_radius_scale aperture_samples timing_iterations output.png\n"
               "e.g.: ./process input.png 32 13 0.5 32 3 output.png\n");
        return 0;
    }

    // Let the Halide runtime hold onto GPU allocations for
    // intermediates and reuse them instead of eagerly freeing
    // them. cuMemAlloc/cuMemFree is slower than the algorithm!
    (void)halide_reuse_device_allocations(nullptr, true);  // ignore error: this function will always succeed when second arg is true

    // Buffer<uint8_t, 3> left_im = load_image(argv[1]);
    // Buffer<uint8_t, 3> right_im = load_image(argv[1]);
    int matrix_size = atoi(argv[1]);
    int rgb = 3;
    Buffer<uint8_t, 3> left_im(matrix_size, matrix_size, rgb);
    Buffer<uint8_t, 3> right_im(matrix_size, matrix_size, rgb);

    // Initialize gradient images
    for (int z = 0; z < rgb; z++) {
        for (int iy = 0; iy < matrix_size; iy++) {
            for (int ix = 0; ix < matrix_size; ix++) {
                left_im(ix, iy, z) = static_cast<uint8_t>((ix + iy + z) % 256);
                right_im(ix, iy, z) = static_cast<uint8_t>((ix + iy + z) % 256);
            }
        }
    }

    uint32_t slices = atoi(argv[2]);
    uint32_t focus_depth = atoi(argv[3]);
    float blur_radius_scale = atof(argv[4]);
    uint32_t aperture_samples = atoi(argv[5]);
    Buffer<float, 3> output(left_im.width(), left_im.height(), 3);
    int timing_iterations = atoi(argv[6]);

    lens_blur(left_im, right_im, slices, focus_depth, blur_radius_scale,
              aperture_samples, output);

    // Timing code

    // Manually-tuned version
    double min_t_manual = benchmark(timing_iterations, 10, [&]() {
        lens_blur(left_im, right_im, slices, focus_depth, blur_radius_scale,
                  aperture_samples, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %gms\n", min_t_manual * 1e3);

    // Auto-scheduled version
    double min_t_auto = benchmark(timing_iterations, 10, [&]() {
        lens_blur_auto_schedule(left_im, right_im, slices, focus_depth,
                                blur_radius_scale, aperture_samples, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %gms\n", min_t_auto * 1e3);

    // convert_and_save_image(output, argv[7]);
    printf("left input\n");
    for (int z = 0; z < rgb; z++) {
        for (int iy = 0; iy < matrix_size; iy++) {
            for (int ix = 0; ix < matrix_size; ix++) {
                printf("%d,",left_im(ix,iy,z));
            }
            printf("\n");
        }
    }
    printf("right_im\n");
    for (int z = 0; z < rgb; z++) {
        for (int iy = 0; iy < matrix_size; iy++) {
            for (int ix = 0; ix < matrix_size; ix++) {
                printf("%d,",right_im(ix,iy,z));
            }
            printf("\n");
        }
    }
    printf("output\n");
    for (int z = 0; z < rgb; z++) {
        for (int iy = 0; iy < matrix_size; iy++) {
            for (int ix = 0; ix < matrix_size; ix++) {
                printf("%f,",output(ix,iy,z));
            }
            printf("\n");
        }
    }
    printf("Success!\n");
    return 0;
}
