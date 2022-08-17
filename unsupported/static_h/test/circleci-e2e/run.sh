#!/bin/bash -xe
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Parse command-line arguments
hermes_npm_package=

while (( "$#" )); do
  case $1 in
    --hermes-npm-package)
      shift || (
        echo "Missing NPM package argument"
        exit 1
      )
      hermes_npm_package="$1"
      ;;
    *)
      echo "Unsupported argument: $1"
      exit 1
      ;;
   esac
   shift
done

# Clone a fresh copy of RN main
git clone --depth=1 https://github.com/facebook/react-native

cd react-native
npm install
# Install new Hermes NPM.
npm install "$hermes_npm_package"

# Modify RN Tester app to print a log message at start-up
echo "console.log('Using Hermes: ' + (global.HermesInternal != null));" >> packages/rn-tester/js/RNTesterApp.android.js

# Wait for emulator to boot
adb wait-for-device

# Build and install release app
./gradlew :packages:rn-tester:android:app:installHermesRelease

cd ..

# Start screen recording for debug
adb shell screenrecord /sdcard/screencap.mp4 &
adb_screen_record_pid=$!
function stashScreenCapture() {
  local exit_code=$?
  adb shell killall -s INT screenrecord
  wait $adb_screen_record_pid || true
  adb pull /sdcard/screencap.mp4 /tmp/hermes/output/screencap.mp4
  exit "$exit_code"
}
trap stashScreenCapture EXIT

# Start app + wait to see relevant log message
adb logcat -c || true  # This sometimes fails to clear the "main" log. This is benign.
adb shell am start com.facebook.react.uiapp/.RNTesterActivity
SECONDS=0
until grep 'Using Hermes: true' <(adb logcat -d -s ReactNativeJS:I) ; do
  if ((SECONDS > 30 )) ; then
    echo "Did not see correct log message within 30s"
    adb logcat -d > /tmp/hermes/output/adblog.txt
    tar cfz /tmp/hermes/output/project.tar.gz react-native
    exit 1
  fi
  sleep 2
done

echo "Test passed: Hermes in use in RNTester App"
exit 0
