#ifndef HERMES_BCGEN_HBC_HBCOPTIMIZATIONS_H
#define HERMES_BCGEN_HBC_HBCOPTIMIZATIONS_H

#include <vector>

#include "hermes/IR/IR.h"

#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"

namespace hermes {
namespace hbc {

/// \return a ConsecutiveStringStorage pre-loaded with the strings from the
/// module \p M, in a way optimized for size and to take advantage of the small
/// string index instructions in hbc.
ConsecutiveStringStorage getOrderedStringStorage(Module *M);

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_HBCOPTIMIZATIONS_H
