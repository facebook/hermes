/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <lyra/lyra_exceptions.h>

#include <cstdlib>
#include <exception>
#include <sstream>
#include <typeinfo>

#include <fbjni/detail/Log.h>

namespace facebook {
namespace lyra {

using namespace detail;

namespace {
std::terminate_handler gTerminateHandler;

void logExceptionAndAbort() {
  if (auto ptr = std::current_exception()) {
    FBJNI_LOGE("Uncaught exception: %s", toString(ptr).c_str());
#ifndef _WIN32
    auto trace = getExceptionTraceHolder(ptr);
    if (trace) {
      logStackTrace(getStackTraceSymbols(trace->stackTrace_));
    }
#endif
  }
  if (gTerminateHandler) {
    gTerminateHandler();
  } else {
    FBJNI_LOGF("Uncaught exception and no gTerminateHandler set");
  }
}

const std::vector<InstructionPointer> emptyTrace;
} // namespace

ExceptionTraceHolder::~ExceptionTraceHolder() {}

detail::ExceptionTraceHolder::ExceptionTraceHolder() {
  // TODO(cjhopman): This should be done more safely (i.e. use preallocated space, etc.).
  stackTrace_.reserve(128);
  getStackTrace(stackTrace_, 1);
}


void ensureRegisteredTerminateHandler() {
  static auto initializer = (gTerminateHandler = std::set_terminate(logExceptionAndAbort));
  (void)initializer;
}

const std::vector<InstructionPointer>& getExceptionTrace(std::exception_ptr ptr) {
#ifndef _WIN32
  auto holder = getExceptionTraceHolder(ptr);
  return holder ? holder->stackTrace_ : emptyTrace;
#else
  return emptyTrace;
#endif
}

std::string toString(std::exception_ptr ptr) {
  if (!ptr) {
    return "No exception";
  }

  try {
    std::rethrow_exception(ptr);
  } catch (std::exception& e) {
    std::stringstream ss;
    ss << typeid(e).name() << ": " << e.what();
    return ss.str();
  } catch (...) {
    return "Unknown exception";
  }
}

}
}
