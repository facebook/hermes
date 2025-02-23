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

  // Get Object.defineProperty(), use it to define property below.
  auto definePropertyFn = hrt.global()
                              .getProperty(hrt, "Object")
                              .asObject(hrt)
                              .getProperty(hrt, "defineProperty")
                              .asObject(hrt)
                              .asFunction(hrt);

  facebook::jsi::Object descriptor{hrt};
  descriptor.setProperty(hrt, "enumerable", false);
  auto valueProp = facebook::jsi::PropNameID::forAscii(hrt, "value");

  // Define $262.global.
  descriptor.setProperty(hrt, valueProp, global);
  definePropertyFn.call(hrt, test262Obj, "global", descriptor);

  // Define $262.evalScript.
  auto evalFunc = global.getProperty(hrt, "eval");
  descriptor.setProperty(hrt, valueProp, evalFunc);
  definePropertyFn.call(hrt, test262Obj, "evalScript", descriptor);

  // Define $262.detachArrayBuffer.
  auto hermesInternalProp = global.getProperty(hrt, "HermesInternal");
  if (hermesInternalProp.isObject()) {
    auto hermesInternalObj = hermesInternalProp.asObject(hrt);
    auto detachArrayBufferFunc =
        hermesInternalObj.getProperty(hrt, "detachArrayBuffer");
    if (detachArrayBufferFunc.isObject()) {
      descriptor.setProperty(hrt, valueProp, detachArrayBufferFunc);
      definePropertyFn.call(hrt, test262Obj, "detachArrayBuffer", descriptor);
    }
  }

  // Define global object $262.
  descriptor.setProperty(hrt, valueProp, test262Obj);
  definePropertyFn.call(hrt, global, "$262", descriptor);

  // Define global function alert().
  auto printFunc = global.getProperty(hrt, "print");
  descriptor.setProperty(hrt, valueProp, printFunc);
  definePropertyFn.call(hrt, global, "alert", descriptor);

  // Define console.log.
  facebook::jsi::Object consoleObj{hrt};
  definePropertyFn.call(hrt, consoleObj, "log", descriptor);
  descriptor.setProperty(hrt, valueProp, consoleObj);
  definePropertyFn.call(hrt, global, "console", descriptor);
}

extern "C" SHERMES_EXPORT void init_console_bindings(SHRuntime *shr) {
  using namespace facebook;
  auto &hrt = *_sh_get_hermes_runtime(shr);
  initTest262Bindings(hrt);
}
