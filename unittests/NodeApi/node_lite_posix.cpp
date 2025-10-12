// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <dlfcn.h>
#include "node_lite.h"

namespace node_api_tests {

//=============================================================================
// NodeLitePlatform implementation
//=============================================================================

/*static*/ void* NodeLitePlatform::LoadFunction(
    napi_env env,
    const std::filesystem::path& lib_path,
    const std::string& function_name) noexcept {
  void* library_handle = dlopen(lib_path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
  if (library_handle == nullptr) {
    const char* error_message = dlerror();
    NODE_LITE_ASSERT(false,
                     "Failed to load dynamic library: %s. Error: %s",
                     lib_path.c_str(),
                     error_message != nullptr ? error_message : "Unknown error");
    return nullptr;
  }

  dlerror(); // Clear any existing error state before dlsym.
  void* symbol = dlsym(library_handle, function_name.c_str());
  const char* error_message = dlerror();
  NODE_LITE_ASSERT(error_message == nullptr,
                   "Failed to resolve symbol: %s in %s. Error: %s",
                   function_name.c_str(),
                   lib_path.c_str(),
                   error_message != nullptr ? error_message : "Unknown error");
  return symbol;
}

}  // namespace node_api_tests
