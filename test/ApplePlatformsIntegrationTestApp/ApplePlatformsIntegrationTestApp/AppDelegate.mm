#import "AppDelegate.h"

#import <jsi/jsi.h>
#import <hermes/hermes.h>

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
