#ifndef HERMES_BCGEN_CPP_H
#define HERMES_BCGEN_CPP_H

#ifdef HERMES_CPP_BACKEND

#include "hermes/IR/IR.h"
#include "hermes/Utils/Options.h"

namespace hermes {
namespace cpp {

/// Given a module \p M and an ostream \p OS, compiles the module into HermesVM
/// c++, and outputs the code to the ostream.
/// \p standalone specifies whether or not headers and main function should be
/// added to make the code directly compileable.
void generateCpp(
    Module *M,
    llvm::raw_ostream &OS,
    bool standalone,
    const BytecodeGenerationOptions &options);

} // namespace cpp
} // namespace hermes

#endif

#endif
