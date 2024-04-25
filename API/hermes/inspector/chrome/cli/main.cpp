/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

#include <getopt.h>
#include <limits.h>

#include <hermes/hermes.h>
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/CDPHandler.h>

namespace fbhermes = ::facebook::hermes;
using CDPHandler = fbhermes::inspector_modern::chrome::CDPHandler;

static const char *usageMessage = R"(hermes-chrome-debug-server script.js

Uses Hermes to evaluate script.js within a debugging session. The process will
wait for Chrome DevTools Protocol requests on stdin and writes responses and
events to stdout.

This can be used with a WebSocket bridge to host a Chrome DevTools Protocol
debug server. For instance, running this:

  websocketd --port=9999 hermes-chrome-debug-server script.js

will run a WebSocket server on port 9999 that debugs script.js in Hermes. Chrome
can connect to this debugging session using a URL like this:

  devtools://devtools/bundled/inspector.html?experiments=false&v8only=true&ws=127.0.0.1:9999

Options:

 -l, --log:  path to a file with pretty-printed protocol logs
 -h, --help: this message
)";

static void usage() {
  fputs(usageMessage, stderr);
  exit(1);
}

/// Print a UTF8 string \p s to a length of at most \p len bytes. If a
/// multi-byte sequence crosses the limit (len), it will be wholly omitted
// from the output. If the output is truncated, it will be suffixed with "...".
static void printTruncated(const std::string &s, size_t len, FILE *logFile) {
  if (s.size() > len) {
    // Iterate back from the edge to pop off continuation bytes.
    while (len > 0 && (s[len] & 0xC0) == 0x80)
      --len;
  } else {
    len = s.size();
  }

  fwrite(s.c_str(), sizeof(s[0]), len, logFile);

  if (len < s.size()) {
    static constexpr char suffix[] = "...";
    fwrite(suffix, sizeof(suffix[0]), sizeof(suffix) - 1, logFile);
  }
}

static FILE *logFile = stderr;

static void setLogFilePath(const char *path) {
  logFile = fopen(path, "w");

  if (logFile == nullptr) {
    perror("fopen couldn't open log file");
    exit(1);
  }
}

static void log(const std::string &str, bool isReq) {
  constexpr size_t MAX_LINE_LEN = 512;
  fprintf(logFile, "%s ", isReq ? "=>" : "<=");
  printTruncated(str, MAX_LINE_LEN, logFile);
  fprintf(logFile, "\n\n");
}

static void logRequest(const std::string &str) {
  log(str, true);
}

static void sendResponse(const std::string &str) {
  log(str, false);
  printf("%s\n", str.c_str());
}

static std::string readScriptSource(const char *path) {
  std::ifstream stream(path);
  return std::string{
      std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

static std::string getUrl(const char *path) {
  char absPath[PATH_MAX] = {};
  if (!realpath(path, absPath)) {
    return "";
  }
  return std::string("file://") + absPath;
}

static void runDebuggerLoop(std::shared_ptr<CDPHandler> cdpHandler) {
  cdpHandler->registerCallbacks(&sendResponse, {});

  std::string line;
  while (std::getline(std::cin, line)) {
    logRequest(line);
    cdpHandler->handle(line);
  }
}

static void runScript(const std::string &scriptSource, const std::string &url) {
  std::shared_ptr<fbhermes::HermesRuntime> runtime(
      fbhermes::makeHermesRuntime(::hermes::vm::RuntimeConfig::Builder()
                                      .withEnableSampleProfiling(true)
                                      .build()));
  auto adapter =
      std::make_unique<fbhermes::inspector_modern::SharedRuntimeAdapter>(
          runtime);
  std::shared_ptr<CDPHandler> cdpHandler =
      CDPHandler::create(std::move(adapter));
  std::thread debuggerLoop(runDebuggerLoop, std::ref(cdpHandler));

  fbhermes::HermesRuntime::DebugFlags flags{};
  runtime->debugJavaScript(scriptSource, url, flags);

  debuggerLoop.join();
}

int main(int argc, char **argv) {
  const char *shortOpts = "l:h";
  const option longOpts[] = {
      {"log", 1, nullptr, 'l'},
      {"help", 0, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}};

  while (true) {
    int opt = getopt_long(argc, argv, shortOpts, longOpts, nullptr);
    if (opt == -1) {
      break;
    }

    switch (opt) {
      case 'l':
        setLogFilePath(optarg);
        break;
      case 'h':
        usage();
        break;
      default:
        fprintf(stderr, "Unrecognized option: %c\n", opt);
        usage();
        break;
    }
  }

  setbuf(logFile, nullptr);
  setbuf(stdout, nullptr);

  if (optind + 1 != argc) {
    usage();
  }

  const char *path = argv[optind];
  std::string scriptSource = readScriptSource(path);
  std::string url = getUrl(path);

  runScript(scriptSource, url);

  fclose(logFile);

  return 0;
}
