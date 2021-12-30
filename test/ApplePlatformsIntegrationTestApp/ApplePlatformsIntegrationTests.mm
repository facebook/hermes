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

@end
