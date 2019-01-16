#ifdef HERMES_FACEBOOK_BUILD
#define DEBUG_TYPE "gc"
#include "hermes/VM/GC.h"

#if defined(__ANDROID__)
#include "glue/BreakpadCustomData.h"
#endif

namespace hermes {
namespace vm {

/*static*/
void GCBase::oomDetailFB(char *detail) {
#if defined(__ANDROID__)
  setBreakpadCustomData("HermesGCOOMDetailBasic", "%s", detail);
#endif
}

} // namespace vm
} // namespace hermes
#endif
