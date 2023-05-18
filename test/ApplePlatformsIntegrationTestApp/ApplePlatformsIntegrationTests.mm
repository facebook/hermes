/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import <XCTest/XCTest.h>

#import <hermes/DebuggerAPI.h>
#import <hermes/hermes.h>

#include <iostream>

using namespace std;

@interface ApplePlatformsIntegrationTests : XCTestCase

@end

@implementation ApplePlatformsIntegrationTests

- (void)testBasicRuntime {
  facebook::hermes::HermesRuntime::DebugFlags flags;

  auto runtime = facebook::hermes::makeHermesRuntime();

  string error = "It is not a bug, it is a feature!";

  try {
    runtime->debugJavaScript("throw new Error('" + error + "')", "", flags);
  } catch (const exception &e) {
    XCTAssertTrue(string(e.what()).find(error) != string::npos);
  }
}

- (void)testNumbersAndLoops {
  facebook::hermes::HermesRuntime::DebugFlags flags;

  auto runtime = facebook::hermes::makeHermesRuntime();

  string js = "const n = 0;\n"
              "for (const i = 0; i < 50; i++) {\n"
              "  n = n + i;\n"
              "}\n"
              "throw new Error('' + n);\n";

  try {
    runtime->debugJavaScript(js, "", flags);
  } catch (const exception &e) {
    string errorString = string(e.what());
    XCTAssertTrue(string(e.what()).find("1225") != string::npos);
  }
}

@end
