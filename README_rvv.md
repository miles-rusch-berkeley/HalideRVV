
# Building Halide with Make

### TL;DR

Have llvm-16.0 (or greater) installed and run `make` in the root directory of
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
        -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
        -DLLVM_TARGETS_TO_BUILD="X86;ARM;NVPTX;AArch64;Hexagon;WebAssembly;RISCV" \
        -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_ASSERTIONS=ON \
        -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_BUILD_32_BITS=OFF \
        -DLLVM_ENABLE_RUNTIMES="compiler-rt" \
        -G Ninja -S llvm-project/llvm -B llvm-build
cmake --build llvm-build
cmake --install llvm-build --prefix llvm-install

cmake --build build
cmake --install build --prefix llvm-install
export LLVM_ROOT=$PWD/llvm-install
export LLVM_CONFIG=$LLVM_ROOT/bin/llvm-config
```

Then clone Halide 18.x:
```
git clone --branch v18.0.0 git@github.com:halide/Halide.git
```

To run, you must set the environment variable `RISCV` to a riscv-linux-gnu toolchain:

```
export $RISCV=/path/to/riscv-linux-gnu-tools
cd Halide
HL-TARGET=riscv-64-linux-rvv-vector_bits_512 make test_apps
```