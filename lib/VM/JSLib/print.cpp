/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/VM/Operations.h"
#include "hermes/VM/StringView.h"

#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace vm {

/// Convert all arguments to string and print them followed by new line.
CallResult<HermesValue> print(void *, Runtime *runtime, NativeArgs args) {
  GCScope scope(runtime);
  auto marker = scope.createMarker();
  bool first = true;

  for (Handle<> arg : args.handles()) {
    scope.flushToMarker(marker);
    auto res = toString_RJS(runtime, arg);
    if (res == ExecutionStatus::EXCEPTION)
      return ExecutionStatus::EXCEPTION;

    if (!first)
      llvm::outs() << " ";
    SmallU16String<32> tmp;
    llvm::outs() << StringPrimitive::createStringView(
                        runtime, toHandle(runtime, std::move(*res)))
                        .getUTF16Ref(tmp);
    first = false;
  }

  llvm::outs() << "\n";
  llvm::outs().flush();
  return HermesValue::encodeUndefinedValue();
}

} // namespace vm
} // namespace hermes
