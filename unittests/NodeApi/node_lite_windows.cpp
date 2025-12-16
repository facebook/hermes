// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <windows.h>
#include "node_lite.h"

namespace node_api_tests {

//=============================================================================
// NodeLitePlatform implementation
//=============================================================================

/*static*/ void* NodeLitePlatform::LoadFunction(
    napi_env env,
    const std::filesystem::path& lib_path,
    const std::string& function_name) noexcept {
  HMODULE dll_module = ::LoadLibraryA(lib_path.string().c_str());
  NODE_LITE_ASSERT(dll_module != NULL,
                   "Failed to load DLL: %s. Error: %s",
                   lib_path.c_str(),
                   std::strerror(errno));
  return ::GetProcAddress(dll_module, function_name.c_str());
}
}  // namespace node_api_tests
