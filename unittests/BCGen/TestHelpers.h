#include "hermes/AST/Context.h"

#include <stdint.h>
#include <vector>
#include "llvm/ADT/SmallVector.h"

namespace hermes {

struct TestCompileFlags {
  bool staticBuiltins{false};
};

/// Compile source code \p source into Hermes bytecode, asserting that it can be
/// compiled successfully. \return the bytecode as a vector of bytes.
std::vector<uint8_t> bytecodeForSource(
    const char *source,
    TestCompileFlags flags = TestCompileFlags());

} // namespace hermes
