#ifndef HALIDE_CODEGEN_VULKAN_DEV_H
#define HALIDE_CODEGEN_VULKAN_DEV_H

/** \file
 * Defines the code-generator for producing Vulkan SPIR-V kernel code
 */

#include <memory>

namespace Halide {

struct Target;

namespace Internal {

struct CodeGen_GPU_Dev;

std::unique_ptr<CodeGen_GPU_Dev> new_CodeGen_Vulkan_Dev(const Target &target);

}  // namespace Internal
}  // namespace Halide

#endif
