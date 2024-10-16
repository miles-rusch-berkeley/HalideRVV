// Separable Convolution Generator
#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class SConvRVV : public Halide::Generator<SConvRVV> {
public:
    enum class PickSchedule { Vector,
	                      Scalar  };
    GeneratorParam<PickSchedule> pick_schedule{"pick_schedule",
                                     /* default value */
		                     PickSchedule::Vector,
		                     /* map from names to values */
		                     {{"vector", PickSchedule::Vector},
		                      {"scalar", PickSchedule::Scalar}}};
       	
    GeneratorParam<int> vlen{"vlen", 512 /* default value */};
    GeneratorParam<int> matrix_dim{"matrix_dim", 64 /* default value */};

    Input<Buffer<float>> input{"input", 2};
    Input<Buffer<float>> kernel{"kernel", 1};
    // Input<Buffer<float>> ky{"ky", 1};
    Output<Buffer<float>> output{"output", 2};
    
    Var x{"x"},  y{"y"};

    void generate() {
        Halide::RDom r(-1,3);

        Func k{"k"};
        k(x) = 0.0f;
        k(-1) = kernel(0);
        k(0) = kernel(1);
        k(1) = kernel(2);
        
        Func rows("rows");
        Func cols("cols");
        Func inb("inb");

        //define blur function
        inb(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        cols(x,y) = sum(inb(x+r.x, y) * k(r.x));

        rows(x,y) = sum(cols(x, y+r.x) * k(r.x));
        output(x, y) = rows(x, y);
        
	//schedule

        if (using_autoscheduler()) {
            // blank
                input.set_estimates({{0, 2592}, {0, 1968}});
                kernel.set_estimates({{3, 3}});
                output.set_estimates({{0, 2592}, {0, 1968}});

        } else {
                const int sew = 8*sizeof(float);
                int nlanes = vlen/sew;
                if(pick_schedule == PickSchedule::Scalar){
                        printf("using SCALAR schedule\n");
                }
                else if (pick_schedule == PickSchedule::Vector){
                        printf("using VECTOR schedule\n");
                        Var xi("xi"), xo("xo");
                        Var yi("yi"), yo("yo");
                        
                        output.bound(x, 0, matrix_dim);
                        output.bound(y, 0, matrix_dim);
                        
                        k.compute_root();
                        k.store_root();

                        output.split(x, xo, xi, nlanes);
                        output.reorder(y,xo);
                        output.vectorize(xi);
                        
                        cols.compute_at(output, y);
                        // // cols.reorder(x, r.x);
                        cols.vectorize(x, nlanes);
                        // cols.compute_root();
                        printf("cols compute root:\n");
                        output.print_loop_nest();
                }
        }
    }
};

HALIDE_REGISTER_GENERATOR(SConvRVV, sconv_rvv)
