#!/bin/sh

XCODEBUILD="xcodebuild -workspace ApplePlatformsIntegrationTestApp.xcworkspace -configuration Debug -scheme ApplePlatformsIntegrationTestApp"

# Build
pod install
$XCODEBUILD
# Get path to product
PRODUCT=$($XCODEBUILD -showBuildSettings | grep -m 1 "BUILT_PRODUCTS_DIR" | grep -oEi "\/.*")
# Launch
OUTPUT=$($PRODUCT/ApplePlatformsIntegrationTestApp.app/Contents/MacOS/ApplePlatformsIntegrationTestApp)
# Test
EXPECTED_OUTPUT="Hello world -- son of Maia and Zeus"
if [[ $OUTPUT != $EXPECTED_OUTPUT ]]; then
  echo "Expected output to be '$EXPECTED_OUTPUT', but got '$OUTPUT'"
  echo "** TEST FAILED **"
  exit 1
else
  echo $OUTPUT
  echo "** TEST SUCCEEDED **"
  exit 0
fi
