//
//  AppDelegate.m
//  ApplePlatformsIntegrationTestMobileApp
//
//  Created by Michał Grabowski on 17/08/2020.
//

#import "AppDelegate.h"

#import <hermes/hermes.h>
#import <jsi/jsi.h>

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    std::string s = "print('Hello world -- son of Maia and Zeus');";
    std::string url = "http://example/js";
    facebook::hermes::HermesRuntime::DebugFlags flags;
    auto runtime = facebook::hermes::makeHermesRuntime();
    runtime->debugJavaScript(s, url, flags);
    exit(0);
    return YES;
}

@end
