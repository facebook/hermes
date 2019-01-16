#include <hermes/hermes.h>
#include <jsi/testlib.h>

using namespace facebook::hermes;

namespace facebook {
namespace jsi {

std::vector<RuntimeFactory> runtimeGenerators() {
  return {
      [] { return makeHermesRuntime(); },
      [] { return makeThreadSafeHermesRuntime(); },
  };
}

} // namespace jsi
} // namespace facebook
