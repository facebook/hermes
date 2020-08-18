#import <XCTest/XCTest.h>

#import <hermes/hermes.h>
#import <hermes/DebuggerAPI.h>

#include <iostream>

using namespace std;

@interface ApplePlatformsIntegrationMobileTests : XCTestCase

@end

@implementation ApplePlatformsIntegrationMobileTests

- (void)testBasicRuntime {
    facebook::hermes::HermesRuntime::DebugFlags flags;
    
    auto runtime = facebook::hermes::makeHermesRuntime();
    
    string error = "It is not a bug, it is a feature!";
    
    try {
        runtime->debugJavaScript("throw new Error('" + error + "')", "", flags);
    } catch (const exception& e) {
        XCTAssertTrue(string(e.what()).find(error) != string::npos);
    }
}

@end
