/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/PerfSection.h"

#if defined(HERMES_FACEBOOK_BUILD) || defined(HERMESVM_PLATFORM_LOGGING)

#ifdef HERMES_FACEBOOK_BUILD
#include "fbsystrace.h"
#include "fbsystrace_tags.h"
#endif

#include "hermes/Platform/Logging.h"
#include "hermes/Support/SNPrintfBuf.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

namespace hermes {

using KVPair = std::pair<std::string, std::string>;

PerfSection::PerfSection(const char *name, const char *category)
    :
#ifdef HERMESVM_PLATFORM_LOGGING
      name_(name),
      category_(category),
#endif
      argValues_(0) {
#ifdef HERMES_FACEBOOK_BUILD
  fbsystrace_begin_section(TRACE_TAG_JS_VM, name);
#endif
}

void PerfSection::addArg(const char *argName, size_t value) {
  // Note that if the argument has already been set, we overwrite it.
  freeDataIfExists(argName);
  auto &val = argValues_[argName];
  val.type = ArgType::SIZE_T;
  val.value.sz_t = value;
}

void PerfSection::addArgD(const char *argName, double d) {
  // Note that if the argument has already been set, we overwrite it.
  freeDataIfExists(argName);
  auto &val = argValues_[argName];
  val.type = ArgType::DOUBLE;
  val.value.d = d;
}

void PerfSection::addArg(
    const char *argName,
    const llvm::StringRef value,
    bool doCopy) {
  freeDataIfExists(argName);
  auto &val = argValues_[argName];
  val.type = ArgType::STRINGREF;

  const char *data;
  if (doCopy) {
    char *copy = new char[value.size()];
    std::memcpy(copy, value.data(), value.size());
    data = copy;
  } else {
    data = value.data();
  }
  val.value.stringref = {.data = data, .size = value.size(), .owned = doCopy};
}

PerfSection::~PerfSection() {
  if (argValues_.empty()) {
#ifdef HERMES_FACEBOOK_BUILD
    fbsystrace_end_section(TRACE_TAG_JS_VM);
#endif
    return;
  }

  // First, transform the ArgValue parts of argValues_ into strings.
  std::vector<KVPair> kvPairs(argValues_.size());
  std::transform(
      argValues_.begin(),
      argValues_.end(),
      kvPairs.begin(),
      [](const std::pair<std::string, ArgValue> &pair) -> KVPair {
        // If more elements are added to ArgType, more cases must be added to
        // this switch.
        switch (pair.second.type) {
          case ArgType::SIZE_T: {
            SNPrintfBuf buf(20);
            buf.printf("%zu", pair.second.value.sz_t);
            // Note that KVPair has std::string elements, so the
            // c_str() is copied.
            return KVPair{pair.first, buf.c_str()};
          }
          case ArgType::DOUBLE: {
            SNPrintfBuf buf(20);
            buf.printf("%f", pair.second.value.d);
            // Note that KVPair has std::string elements, so the
            // c_str() is copied.
            return KVPair{pair.first, buf.c_str()};
          }
          case ArgType::STRINGREF:
            return KVPair{pair.first,
                          std::string(
                              pair.second.value.stringref.data,
                              pair.second.value.stringref.size)};
          default:
            llvm_unreachable("Unhandled argtype");
        }
      });
#ifdef HERMES_FACEBOOK_BUILD
  // To output via fbsystrace, we must further translate into
  // FBSystraceSectionArg format.
  auto argArr = std::unique_ptr<FbSystraceSectionArg[]>{
      new FbSystraceSectionArg[argValues_.size()]};
  std::transform(
      kvPairs.begin(), kvPairs.end(), argArr.get(), [](const KVPair &kvPair) {
        return FbSystraceSectionArg{
            .key = kvPair.first.c_str(),
            .key_len = static_cast<int>(kvPair.first.size()),
            .value = kvPair.second.c_str(),
            .value_len = static_cast<int>(kvPair.second.size()),
        };
      });
  fbsystrace_end_section_with_args(
      TRACE_TAG_JS_VM, argValues_.size(), argArr.get());
#endif
#ifdef HERMESVM_PLATFORM_LOGGING
  SNPrintfBuf buf(1000);
  bool first = true;
  for (const KVPair &kvPair : kvPairs) {
    if (first) {
      first = false;
    } else {
      buf.printf(", ");
    }
    buf.printf("%s: %s", kvPair.first.c_str(), kvPair.second.c_str());
  }
  hermesLog(
      "HermesGC",
      "%s[%s]: %s.",
      name_,
      (category_ ? category_ : ""),
      buf.c_str());
#endif
  for (auto &arg : argValues_) {
    arg.second.freeDependencies();
  }
}

} // namespace hermes

#endif // defined(HERMES_FACEBOOK_BUILD) ||
       // defined(HERMESVM_PLATFORM_LOGGING)
