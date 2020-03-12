#!/bin/bash -xe
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Parse command-line arguments
use_react_native_master=
hermes_npm_package=
artifact_postfix=
project_dir=awesome1

while (( "$#" )); do
  case $1 in
    --react-native-master)
      use_react_native_master=1
      #artifact_postfix=rn-master
      #project_dir=awesome2
      ;;
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

# Perform GNU sed operation confirming file changed
function sed_op() {
  local e=$1
  local file=$2
  # Use GNU version of sed if installed by Brew on MacOS
  if [[ -x /usr/local/bin/gsed ]] ; then
    sed_exec="/usr/local/bin/gsed"
  else
    sed_exec="sed"
  fi
  local hash_before
  hash_before=$(md5sum "$file")
  $sed_exec -i "$e" "$file"
  # This will exit with failure if the file didn't change due to -e
  [[ "$(md5sum "$file")" != "$hash_before" ]]
}

npx react-native init "$project_dir"

# Enable Hermes
# https://reactnative.dev/docs/hermes
sed_op 's/enableHermes: false/enableHermes: true/' "$project_dir"/android/app/build.gradle

# Patch in master React Native checkout.
# https://github.com/facebook/react-native/wiki/Building-from-source
if [[ -n "$use_react_native_master" ]] ; then
  pushd "$project_dir"/node_modules/
  mv react-native react-native-
  git clone https://github.com/facebook/react-native
  cd react-native
  npm install
  popd
  sed_op '/classpath("com.android.tools.build:gradle:3.4.2")/aclasspath("de.undercouch:gradle-download-task:4.0.0")' "$project_dir"/android/build.gradle
  cat >> "$project_dir"/android/settings.gradle <<EOT
include ':ReactAndroid'
project(':ReactAndroid').projectDir = new File(
    rootProject.projectDir, '../node_modules/react-native/ReactAndroid')
EOT
  sed_op 's/implementation "com.facebook.react:react-native:.*/implementation project(":ReactAndroid")/' "$project_dir"/android/app/build.gradle
fi

# Add newly built Hermes package
if [[ -n "$hermes_npm_package" ]] ; then
  ( cd "$project_dir"/node_modules/react-native && npm install "$hermes_npm_package" )
fi

# Modify app to print a log message at start-up
echo "console.log('Using Hermes: ' + (global.HermesInternal != null));" >> "$project_dir"/index.js

# Wait for emulator to boot
adb wait-for-device

# Build APK + install onto emulator
( cd "$project_dir"/android/ && ./gradlew installRelease )

# Stash the APK
cp "$project_dir"/android/app/build/outputs/apk/release/app-release.apk /tmp/app-release-"$artifact_postfix".apk

# Start screen recording for debug
adb shell screenrecord /sdcard/screencap.mp4 &
adb_screen_record_pid=$!
function stashScreenCapture() {
  local exit_code=$?
  adb shell killall -s INT screenrecord
  wait $adb_screen_record_pid || true
  adb pull /sdcard/screencap.mp4 /tmp/screencap-"$artifact_postfix".mp4
  exit "$exit_code"
}
trap stashScreenCapture EXIT

# Start app + wait to see relevant log message
adb logcat -c || true  # This sometimes fails to clear the "main" log. This is benign.
adb shell am start com."$project_dir"/.MainActivity
SECONDS=0
until grep 'Using Hermes: true' <(adb logcat -d -s ReactNativeJS:I) ; do
  if ((SECONDS > 30 )) ; then
    echo "Did not see correct log message within 30s"
    exit 1
  fi
  sleep 2
done

echo "Test passed: Hermes in use in React Native Awesome App"
exit 0
