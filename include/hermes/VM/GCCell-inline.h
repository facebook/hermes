#include "hermes/VM/GCCell.h"

#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/StringPrimitive.h"
#ifdef UNIT_TEST
#include "ExtString.h"
#endif

namespace hermes {
namespace vm {

inline uint32_t GCCell::externalMemorySize() const {
  // TODO (T27363944): a more general way of doing this, if we ever have more
  // gc kinds with external memory charges.
  return StringPrimitive::externalMemorySize(this) +
      JSArrayBuffer::externalMemorySize(this);
}

} // namespace vm
} // namespace hermes
