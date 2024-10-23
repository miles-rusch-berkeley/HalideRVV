# Building Halide for RISCV Targets with Make

### TL;DR

Have llvm-18 installed and run 
```
HL-TARGET=riscv-64-linux-rvv-vector_bits_256 make trace_apps
``` 
in the root directory of
the repository (where this README is).

### Acquiring LLVM

At any point in time, building Halide requires either the latest stable version
of LLVM, the previous stable version of LLVM, and trunk. At the time of writing,
this means versions 18, 17, and 16 are supported, but 15 is not. The commands
`llvm-config` and `clang` must be somewhere in the path.

If your OS does not have packages for LLVM, you can find binaries for it at
http://llvm.org/releases/download.html. Download an appropriate package and then
either install it, or at least put the `bin` subdirectory in your path. (This
works well on OS X and Ubuntu.)

If you want to build it yourself, first check it out from GitHub:

```
% git clone --depth 1 --branch llvmorg-18 https://github.com/llvm/llvm-project.git
```

(If you want to build LLVM 17.x, use branch `release/17.x`; for current trunk,
use `main`)

Then build it like so:

```
cmake -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS="clang;lld" \
        -DLLVM_TARGETS_TO_BUILD="X86;ARM;WebAssembly;RISCV" \
        -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_ASSERTIONS=ON \
        -DLLVM_ENABLE_EH=OFF \
        -DLLVM_ENABLE_RTTI=OFF \
        -DLLVM_ENABLE_HTTPLIB=OFF \
        -DLLVM_ENABLE_LIBEDIT=OFF \
        -DLLVM_ENABLE_LIBXML2=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_ENABLE_ZSTD=OFF \
        -DLLVM_BUILD_32_BITS=OFF \
        -DLLVM_ENABLE_RUNTIMES="compiler-rt" \
        -G Ninja -S llvm-project/llvm -B llvm-build
cmake --build llvm-build
cmake --install llvm-build --prefix llvm-install
```

Then, point Halide to it:

```
export LLVM_ROOT=$PWD/llvm-install
export LLVM_CONFIG=$LLVM_ROOT/bin/llvm-config
```

Then clone Halide 18.x:
```
git clone --branch v18.0.0 git@github.com:halide/Halide.git
cd Halide
```

### Building Halide with make

With `LLVM_CONFIG` set (or `llvm-config` in your path), you should be able to
just run `make` in the root directory of the Halide source tree.
`make test_apps` will compile and run all the apps for the `host` target (but won't check their output).

To run the apps for RISC-V targets, use `make` and specify the desired `HL-Target`. The `RISCV` environment variable must be set to a riscv-linux-gnu toolchain (including `pk`). A dynamic instruction trace can be generated for some apps with `make trace_apps` like so:

```
export RISCV=/path/to/riscv-linux-gnu-tools
HL-TARGET=riscv-64-linux-rvv-vector_bits_256 make trace_apps
```