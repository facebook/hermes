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

#include <lyra/lyra.h>

namespace facebook {
namespace lyra {

/**
 * This can be overridden by an implementation capable of looking up
 * the breakpad id for logging purposes.
 */
#ifndef _MSC_VER
__attribute__((weak))
#endif
std::string getBreakpadId(const std::string& library) {
  return "<unimplemented>";
}

}
}
