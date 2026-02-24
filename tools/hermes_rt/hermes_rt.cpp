/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT license.
 */

/// hermes_rt — lightweight JS test runner that exercises hermes.dll via the
/// public C API (jsr_*/hermes_*/napi_*).  Designed for lit-based integration
/// tests, especially Intl provider testing.
///
/// Usage:
///   hermes_rt [options] <file.js>
///
/// Options:
///   --intl-provider=default      Auto-detect (bundled > system > WinGlob)
///   --intl-provider=winglob      Force Windows NLS APIs, no ICU
///   --intl-provider=system-icu   Force Windows system icu.dll (Win10 1903+)
///   --intl-provider=host-vtable  Load hermes-icu.dll vtable as host-provided
///   --intl-icu-path=<dir>        Directory with external ICU DLLs
///   --intl-icu-version=<N>       External ICU major version (e.g. 78)
///
/// The built-in print() function is provided by the Hermes VM global object,
/// so no Node-API registration is needed.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "hermes_api.h"
#include "js_runtime_api.h"
#include "node_api.h"

#ifdef _WIN32
#include "hermes/Platform/Intl/hermes_icu.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Read an entire file into a std::string.  Returns empty on failure.
static std::string readFile(const char *path) {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) {
    std::cerr << "hermes_rt: cannot open file: " << path << "\n";
    return {};
  }
  std::ostringstream ss;
  ss << ifs.rdbuf();
  return ss.str();
}

/// Print a JS exception to stderr.  Returns 1 (exit code).
static int reportException(napi_env env) {
  bool isPending = false;
  napi_is_exception_pending(env, &isPending);
  if (!isPending)
    return 1;

  napi_value error{};
  napi_get_and_clear_last_exception(env, &error);
  if (!error)
    return 1;

  // Try to extract error.stack (includes name + message + callstack).
  napi_value stackVal{};
  napi_get_named_property(env, error, "stack", &stackVal);

  napi_valuetype stackType = napi_undefined;
  if (stackVal)
    napi_typeof(env, stackVal, &stackType);

  if (stackType == napi_string) {
    size_t len = 0;
    napi_get_value_string_utf8(env, stackVal, nullptr, 0, &len);
    std::string stack(len, '\0');
    napi_get_value_string_utf8(env, stackVal, stack.data(), len + 1, nullptr);
    std::cerr << stack << "\n";
  } else {
    // Fallback: coerce error to string.
    napi_value strVal{};
    napi_coerce_to_string(env, error, &strVal);
    if (strVal) {
      size_t len = 0;
      napi_get_value_string_utf8(env, strVal, nullptr, 0, &len);
      std::string msg(len, '\0');
      napi_get_value_string_utf8(env, strVal, msg.data(), len + 1, nullptr);
      std::cerr << msg << "\n";
    }
  }
  return 1;
}

// ---------------------------------------------------------------------------
// Command-line parsing
// ---------------------------------------------------------------------------

struct Options {
  const char *jsFile = nullptr;
  uint8_t intlProviderMode = 0; // 0 = Default, 1 = ForceWinGlob
  bool useSystemIcu = false; // load system icu.dll explicitly
  bool useHostVtable = false; // load hermes-icu.dll vtable as host-provided
  const char *icuPath = nullptr; // directory containing external ICU DLLs
  uint32_t icuVersion = 0; // external ICU major version (e.g. 78)
};

static void printUsage() {
  std::cerr
      << "Usage: hermes_rt [options] <file.js>\n"
      << "\n"
      << "Options:\n"
      << "  --intl-provider=default      Auto-detect ICU provider (default)\n"
      << "  --intl-provider=winglob      Force Windows NLS APIs, no ICU\n"
      << "  --intl-provider=system-icu   Force Windows system icu.dll\n"
      << "  --intl-provider=host-vtable  Host-provided vtable from "
         "hermes-icu.dll\n"
      << "  --intl-icu-path=<dir>        External ICU DLL directory\n"
      << "  --intl-icu-version=<N>       External ICU major version\n";
}

static bool parseArgs(int argc, char **argv, Options &opts) {
  for (int i = 1; i < argc; ++i) {
    if (strncmp(argv[i], "--intl-provider=", 16) == 0) {
      const char *val = argv[i] + 16;
      if (strcmp(val, "default") == 0) {
        opts.intlProviderMode = 0;
      } else if (strcmp(val, "winglob") == 0) {
        opts.intlProviderMode = 1;
      } else if (strcmp(val, "system-icu") == 0) {
        opts.useSystemIcu = true;
      } else if (strcmp(val, "host-vtable") == 0) {
        opts.useHostVtable = true;
      } else {
        std::cerr << "hermes_rt: unknown intl provider: " << val << "\n";
        return false;
      }
    } else if (strncmp(argv[i], "--intl-icu-path=", 16) == 0) {
      opts.icuPath = argv[i] + 16;
    } else if (strncmp(argv[i], "--intl-icu-version=", 19) == 0) {
      opts.icuVersion = static_cast<uint32_t>(atoi(argv[i] + 19));
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printUsage();
      return false;
    } else if (argv[i][0] == '-') {
      std::cerr << "hermes_rt: unknown option: " << argv[i] << "\n";
      return false;
    } else {
      // First non-option argument is the JS file.
      opts.jsFile = argv[i];
    }
  }

  if (!opts.jsFile) {
    std::cerr << "hermes_rt: no input file\n";
    printUsage();
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char **argv) {
  Options opts;
  if (!parseArgs(argc, argv, opts))
    return 1;

  // Read the JS source file.
  std::string source = readFile(opts.jsFile);
  if (source.empty())
    return 1;

#ifdef _WIN32
  // Host-vtable mode: load hermes-icu.dll and provide its vtable as
  // a host-provided custom vtable (mode 2 / CustomVtable).
  static hermes_icu_vtable hostVtableCopy = {};
  if (opts.useHostVtable) {
    // Find hermes-icu.dll next to this executable.
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wchar_t *lastSep = wcsrchr(exePath, L'\\');
    if (!lastSep) {
      std::cerr << "hermes_rt: cannot determine exe directory\n";
      return 1;
    }
    wcscpy_s(lastSep + 1,
        MAX_PATH - static_cast<size_t>(lastSep + 1 - exePath),
        L"hermes-icu.dll");

    HMODULE icuDll = LoadLibraryExW(
        exePath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!icuDll) {
      std::cerr << "hermes_rt: failed to load hermes-icu.dll\n";
      return 1;
    }

    using GetVtableFn = const hermes_icu_vtable *(*)();
    auto proc = GetProcAddress(icuDll, "hermes_icu_get_vtable");
    auto getVtable = reinterpret_cast<GetVtableFn>(
        reinterpret_cast<void *>(proc));
    if (!getVtable) {
      std::cerr << "hermes_rt: hermes_icu_get_vtable not found\n";
      return 1;
    }

    const hermes_icu_vtable *original = getVtable();
    if (!original || original->version != HERMES_ICU_VTABLE_VERSION) {
      std::cerr << "hermes_rt: vtable version mismatch\n";
      return 1;
    }

    // Copy and modify metadata to prove host vtable path works.
    memcpy(&hostVtableCopy, original, sizeof(hermes_icu_vtable));
    hostVtableCopy.provider_name = "host-test";
  }

  // Load system ICU if requested.  Must happen before runtime creation.
  if (opts.useSystemIcu) {
    wchar_t sysDir[MAX_PATH];
    GetSystemDirectoryW(sysDir, MAX_PATH);
    std::wstring icuPath = std::wstring(sysDir) + L"\\icu.dll";
    int32_t st =
        hermes_icu_load_from_path(icuPath.c_str(), nullptr, 0);
    if (st != 0) {
      std::cerr << "hermes_rt: failed to load system icu.dll\n";
      return 1;
    }
  }

  // Load external ICU DLLs if requested.  Must happen before runtime creation.
  if (opts.icuPath) {
    // Convert path to wide string.
    int wlen = MultiByteToWideChar(
        CP_UTF8, 0, opts.icuPath, -1, nullptr, 0);
    std::wstring wdir(wlen, L'\0');
    MultiByteToWideChar(
        CP_UTF8, 0, opts.icuPath, -1, wdir.data(), wlen);
    // Remove trailing null from wstring.
    if (!wdir.empty() && wdir.back() == L'\0')
      wdir.pop_back();

    // Ensure trailing separator.
    if (!wdir.empty() && wdir.back() != L'\\' && wdir.back() != L'/')
      wdir += L'\\';

    // Build DLL paths: icuuc<ver>.dll, icuin<ver>.dll
    wchar_t commonPath[MAX_PATH], i18nPath[MAX_PATH];
    swprintf_s(commonPath, MAX_PATH, L"%sicuuc%u.dll",
        wdir.c_str(), opts.icuVersion);
    swprintf_s(i18nPath, MAX_PATH, L"%sicuin%u.dll",
        wdir.c_str(), opts.icuVersion);

    int32_t st = hermes_icu_load_from_path(
        commonPath, i18nPath, opts.icuVersion);
    if (st != 0) {
      std::cerr << "hermes_rt: failed to load external ICU from "
                << opts.icuPath << "\n";
      return 1;
    }
  }
#endif

  // Create runtime config.
  jsr_config config{};
  if (jsr_create_config(&config) != napi_ok) {
    std::cerr << "hermes_rt: failed to create config\n";
    return 1;
  }

#ifdef _WIN32
  // Set Intl provider mode.
  if (opts.useHostVtable) {
    hermes_config_set_intl_provider(
        config, 2 /*CustomVtable*/, &hostVtableCopy);
  } else {
    hermes_config_set_intl_provider(config, opts.intlProviderMode, nullptr);
  }
#endif

  // Create runtime.
  jsr_runtime runtime{};
  if (jsr_create_runtime(config, &runtime) != napi_ok) {
    std::cerr << "hermes_rt: failed to create runtime\n";
    jsr_delete_config(config);
    return 1;
  }
  jsr_delete_config(config);

  // Get napi_env.
  napi_env env{};
  if (jsr_runtime_get_node_api_env(runtime, &env) != napi_ok) {
    std::cerr << "hermes_rt: failed to get napi_env\n";
    jsr_delete_runtime(runtime);
    return 1;
  }

  // Execute the script.
  int exitCode = 0;
  {
    napi_value sourceVal{};
    napi_create_string_utf8(
        env, source.c_str(), source.size(), &sourceVal);

    napi_value result{};
    napi_status status =
        jsr_run_script(env, sourceVal, opts.jsFile, &result);

    if (status != napi_ok) {
      exitCode = reportException(env);
    }
  }

  // Flush stdout so lit/FileCheck sees all output.
  fflush(stdout);

  jsr_delete_runtime(runtime);
  return exitCode;
}
