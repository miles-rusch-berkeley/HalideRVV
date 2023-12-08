// Halide tutorial lesson 24: Async execution

// This lesson demonstrates how to asynchronously execute a function
// using scheduling directives 'async' and 'double_buffer'.

// On linux, you can compile and run it like so:
// g++ lesson_24*.cpp -g -I <path/to/Halide.h> -L <path/to/libHalide.so> -lHalide -lpthread -ldl -o lesson_24 -std=c++17
// LD_LIBRARY_PATH=<path/to/libHalide.so> ./lesson_24

// On os x:
// g++ lesson_24*.cpp -g -I <path/to/Halide.h> -L <path/to/libHalide.so> -lHalide -o lesson_24 -std=c++17
// DYLD_LIBRARY_PATH=<path/to/libHalide.dylib> ./lesson_24

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_24_async
// in a shell with the current directory at the top of the halide
// source tree.

#include "Halide.h"
#include <stdio.h>

using namespace Halide;

int main(int argc, char **argv) {
    // Declare some Vars to use below.
    Var x("x"), y("y"), c("c"), xo("xo"), yo("yo"), xi("xi"), yi("yi"), tile("tile");

    {
        // In this example we simply tell Halide to run `producer` in a
        // separate thread. This is not very useful on its own, but is a good start
        // for the next examples.
        Func producer("producer"), consumer("consumer");

        producer(x, y) = x + y;
        consumer(x, y) = producer(x - 1, y - 1) + producer(x, y) + producer(x + 1, y + 1);

        consumer.compute_root();
        // Use async() to produce `producer` in a separate thread.
        producer.compute_root().async();

        // The high-level structure of the generated code will be:
        // {
        //     allocate producer[...]
        //     thread #1 {
        //         produce producer {
        //             ...
        //         }
        //         signal that data is ready
        //     }
        //     thread #2 {
        //         consume producer {
        //             block until producer data is ready       
        //             produce consumer {
        //                 ...                        
        //             }
        //         }
        //     }
        // }
        consumer.realize({128, 128});
    }

    {
        // Now let's use async() to execute two different producers simultaneously.
        // This could be useful in various scenarios when you want to overlap 
        // computations of different functions in time. For example, you could execute 
        // producer1 and producer2 on different devices in parallel (e.g producer1 on CPU
        // and producer2 on GPU).
        Func producer1("producer1"), producer2("producer2"), consumer("consumer");

        producer1(x, y) = x + y;
        producer2(x, y) = x + y;
        consumer(x, y) = producer1(x - 1, y - 1) + producer2(x, y) + producer1(x + 1, y + 1);

        // With the schedule below, `producer1` and `producer2` computations will be each 
        // launched in separate threads. Since `consumer` depends on both of them, and producers
        // are scheduled as compute_root(), `consumer` will have to wait until `producer1` and
        // `producer2` fully completed their work. The required synchronization primitives 
        // will be added between producers and `consumer` to ensure that  it's safe for `consumer`
        // to start its work and input data is fully ready.
        consumer.compute_root();
        producer1.compute_root().async();
        producer2.compute_root().async();

        // The high-level structure of the generated code will be:
        // {
        //     allocate producer1[...]
        //     allocate producer2[...]
        //     thread #1 {
        //         produce producer1 {
        //             ...
        //         }
        //         signal that producer1 data is ready
        //     }
        //     thread #2 {
        //         produce producer2 {
        //             ...
        //         }
        //         signal that producer2 data is ready
        //     }
        //     thread #3 {
        //         consume producer1 {
        //             consume producer2 {
        //                 block until producer1 data is ready
        //                 block until producer2 data is ready
        //                 produce consumer {
        //                     ...                        
        //                 }
        //             }
        //         }
        //     }
        // }
        consumer.realize({128, 128});
    }

    {
        // In the previous example, we managed to run two producers in parallel, but `consumer` had
        // to wait until the data is fully ready. Wouldn't it be great if we could overlap computations
        // of `producer` and `consumer` too? This computational pattern is known as 'double buffering' and
        // can be critical for achieving good performance in certain scenarios. The high-level idea is that
        // producer is allowed to run ahead and do the next chunk of work without waiting while consumer 
        // is processing the current chunk. The obvious drawback of this method is that it requires twice
        // as much memory for `producer`. 
        Func producer("producer"), consumer("consumer");

        producer(x, y, c) = (x + y) * (c + 1);
        consumer(x, y, c) = producer(x - 1, y - 1, c) + producer(x, y, c) + producer(x + 1, y + 1, c);

        consumer.compute_root();

        // In this example the planes are processed separately, so producer can run ahead 
        // and start producing plane `c + 1`, while `consumer` consumes already produced plane `c`.
        // One way to express it with Halide schedule is very similar to how sliding window schedules
        // are expressed (see lesson_8 for details). There are indeed a lot of commonalities between two
        // because both of them are relying on a circular buffer as underlying data structure.
        producer.compute_root()
            .async()
            .compute_at(consumer, c)
            .store_at(consumer, Var::outermost())
            // Explicit fold_storage is required here, because otherwise Halide will infer that only
            // one plane of `producer` is necessary for `consumer`, but for the purposes of this
            // example we want at least 2.
            .fold_storage(c, 2);

        consumer.realize({128, 128, 4});
    }

    {
        // In the previous example, we relied on storage folding to to use double buffering
        // technique, but there is another, more direct way to express that.
        Func producer("producer"), consumer("consumer");

        producer(x, y, c) = (x + y) * (c + 1);
        consumer(x, y, c) = producer(x - 1, y - 1, c) + producer(x, y, c) + producer(x + 1, y + 1, c);

        consumer.compute_root();

        // As mentioned in the previous example, the planes are processed separately, so producer can run
        // ahead and start producing plane `c + 1`, while `consumer` consumes already produced plane `c`.
        // A more direct way to express this would be to hoist storage of `producer` to ouside of the loop
        // `c` over planes, double its size and add necessary indices to flip between two planes.
        // The first part can be achieved with `hoist_storage` directive and the rest is done with 
        // `double_buffer`.
        producer.compute_root()
            .async()
            .compute_at(consumer, c)
            .hoist_storage(consumer, Var::outermost())
            .double_buffer()
            .async();

        consumer.realize({128, 128, 4});
    }

    {
        // The advantage of the `hoist_storage` + `double_buffer` approach is that it can be applied to
        // fairly arbitrary loop splits and tilings. For example, in the following schedule isntead of 
        // double buffering over whole planes, we double buffer over sub-regions or tiles of the planes.
        Func producer("producer"), consumer("consumer");

        producer(x, y, c) = (x + y) * (c + 1);
        consumer(x, y, c) = producer(x - 1, y - 1, c) + producer(x, y, c) + producer(x + 1, y + 1, c);

        consumer.compute_root()
            .tile(x, y, xo, yo, xi, yi, 16, 16, TailStrategy::Auto);

        producer.compute_root()
            .async()
            .compute_at(consumer, xo)
            .hoist_storage(consumer, Var::outermost())
            .double_buffer();

        consumer.realize({128, 128, 4});
    }

    printf("Success!\n");

    return 0;
}
