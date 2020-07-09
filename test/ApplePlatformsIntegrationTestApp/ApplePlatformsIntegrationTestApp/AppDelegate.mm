/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import "AppDelegate.h"

#import <hermes/hermes.h>
#import <jsi/jsi.h>

@interface AppDelegate ()
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  std::string s = "print('Hello world -- son of Maia and Zeus');";
  std::string url = "http://example/js";
  facebook::hermes::HermesRuntime::DebugFlags flags;
  auto runtime = facebook::hermes::makeHermesRuntime();
  runtime->debugJavaScript(s, url, flags);
  exit(0);
}

@end
