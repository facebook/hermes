#include "hermes/VM/OOMException.h"

namespace {
const char *kOOMWhat = "Hermes: Out of Memory";
} // namespace

namespace hermes {
namespace vm {

const char *OOMException::what() const noexcept {
  return kOOMWhat;
}

} // namespace vm
} // namespace hermes
