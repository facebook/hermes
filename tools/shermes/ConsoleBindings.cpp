/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/static_h.h"
#include "hermes/hermes.h"
#include "jsi/jsi.h"

/// Init harness symbols used in the test262 testsuite:
/// - $test262, with properties:
///   - global
///   - evalScript
///   - detachArrayBuffer
/// - alert
static void initTest262Bindings(facebook::hermes::HermesRuntime &hrt) {
  auto global = hrt.global();
  facebook::jsi::Object test262Obj{hrt};

  // Define $262.global.
  test262Obj.setProperty(hrt, "global", global);

  // Define $262.evalScript.
  auto evalFunc = global.getProperty(hrt, "eval");
  test262Obj.setProperty(hrt, "evalScript", evalFunc);

  // Define $262.detachArrayBuffer.
  auto hermesInternalProp = global.getProperty(hrt, "HermesInternal");
  if (hermesInternalProp.isObject()) {
    auto hermesInternalObj = hermesInternalProp.asObject(hrt);
    auto detachArrayBufferFunc =
        hermesInternalObj.getProperty(hrt, "detachArrayBuffer");
    if (detachArrayBufferFunc.isObject()) {
      test262Obj.setProperty(hrt, "detachArrayBuffer", detachArrayBufferFunc);
    }
  }

  // Define global object $262.
  global.setProperty(hrt, "$262", test262Obj);

  // Define global function alert().
  auto printFunc = global.getProperty(hrt, "print");
  global.setProperty(hrt, "alert", printFunc);

  // Define console.log.
  facebook::jsi::Object consoleObj{hrt};
  global.setProperty(hrt, "console", consoleObj);
  consoleObj.setProperty(hrt, "log", printFunc);
}

extern "C" SHERMES_EXPORT void init_console_bindings(SHRuntime *shr) {
  using namespace facebook;
  auto &hrt = *_sh_get_hermes_runtime(shr);
  initTest262Bindings(hrt);
}
