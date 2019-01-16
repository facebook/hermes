#ifdef HERMESVM_SYNTH_REPLAY
#include <hermes/TraceInterpreter.h>
#include <hermes/hermes.h>

#include <gtest/gtest.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>

#include <memory>
#include <string>
#include <vector>

namespace {

using namespace facebook::hermes;
using namespace hermes::vm;

std::string getBaseName(const std::string &path) {
  // Remove all previous slashes.
  auto lastSlashPos = path.find_last_of('/');
  assert(
      lastSlashPos != std::string::npos &&
      "There must be a slash in the path name");
  auto lastDotPos = path.find_last_of('.');
  if (lastDotPos == std::string::npos || lastDotPos < lastSlashPos) {
    // If the last dot doesn't exist or is before the last slash, it has no
    // extension.
    return path.substr(lastSlashPos + 1);
  } else {
    return path.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
  }
}

std::unordered_map<std::string, std::string> getFiles(
    const std::string &dirPath) {
  std::error_code ec;
  std::unordered_map<std::string, std::string> files;
  for (llvm::sys::fs::directory_iterator entry(dirPath, ec);
       entry != llvm::sys::fs::directory_iterator();
       entry.increment(ec)) {
    if (ec) {
      throw std::system_error(ec);
    }
    const auto path = entry->path();
    files.emplace(getBaseName(path), path);
  }
  return files;
}

void runInterpreter(
    const std::string &traceFile,
    const std::string &bytecodeFile) {
  // This will fail with an exception for an error.
  std::string outTrace;
  llvm::raw_string_ostream outTraceStream{outTrace};
  TraceInterpreter::ExecuteOptions options;
  // Use value in the trace.
  options.minHeapSize = 0;
  options.maxHeapSize = 0;
  TraceInterpreter::execAndTrace(
      traceFile, bytecodeFile, options, outTraceStream);
  // Do nothing with outTrace. It could be compared to the original trace,
  // but it requires normalization.
}

TEST(SynthBenchmarkTest, Run) {
  const char *bundlesDir = std::getenv("BUNDLE_FILES_TO_TEST");
  const char *tracesDir = std::getenv("TRACE_FILES_TO_TEST");
  if (!bundlesDir) {
    FAIL() << "No bundles directory supplied in BUNDLE_FILES_TO_TEST";
  }
  if (!tracesDir) {
    FAIL() << "No traces directory supplied in TRACE_FILES_TO_TEST";
  }
  const auto bundles = getFiles(bundlesDir);
  const auto traces = getFiles(tracesDir);
  if (bundles.empty()) {
    FAIL() << "No bundle files found to test";
  }
  if (traces.empty()) {
    FAIL() << "No trace files found to test";
  }
  for (const auto &p : bundles) {
    const auto &testName = p.first;
    const auto &bundle = p.second;
    const auto &trace = traces.at(testName);
    if (bundle.empty()) {
      FAIL() << "Bundle is null for: " << testName;
    }
    if (trace.empty()) {
      FAIL() << "Trace is null for: " << testName;
    }
    EXPECT_NO_THROW({ runInterpreter(trace, bundle); })
        << "Failed on file: " << testName;
  }
}

} // namespace
#endif
