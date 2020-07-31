#!/bin/bash -xe
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Parse command-line arguments
use_react_native_master=
hermes_npm_package=
project_dir=awesome1

while (( "$#" )); do
  case $1 in
    --react-native-master)
      use_react_native_master=1
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

# Use md5 instead of md5sum if the latter can't be found
if ! command -v md5sum > /dev/null
then
  md5sum() {
    md5 "$@"
  }
fi

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

project_path="$PWD/$project_dir"
commit() {
  ( cd "$project_path" && git commit -a -m "$1" )
}

npx react-native init "$project_dir"

# Install new NPM. This must be done before we start messing with
# node_modules/react-native
if [[ -n "$hermes_npm_package" ]]
then
  ( cd "$project_path" && npm install "$hermes_npm_package" )
fi

( cd "$project_path" && git init && git add . )
commit "Initial project"

# The template project from RN 0.63 is no longer compatible with RN master.  We
# should really be using the RN master template, but the CLI functionality for
# this is currently broken. Meanwhile, we'll just kludge the required changes.
#
# https://github.com/react-native-community/cli/pull/1110
#
sed_op 's/minSdkVersion = .*/minSdkVersion = 19/' "$project_dir"/android/build.gradle
commit "Kludge minSdkVersion until we can use an updated RN project"

# Enable Hermes
# https://reactnative.dev/docs/hermes
sed_op 's/enableHermes: false/enableHermes: true/' "$project_dir"/android/app/build.gradle
commit "Enable hermes"

# Patch in master React Native checkout.
# https://github.com/facebook/react-native/wiki/Building-from-source
if [[ -n "$use_react_native_master" ]] ; then
  pushd "$project_dir"/node_modules/
  mv react-native react-native-
  git clone --depth=1 https://github.com/facebook/react-native
  cd react-native
  npm install
  [[ -n "$hermes_npm_package" ]] && npm install "$hermes_npm_package"
  popd
  sed_op '/classpath("com.android.tools.build:gradle:.*")/aclasspath("de.undercouch:gradle-download-task:4.0.0")' "$project_dir"/android/build.gradle
  cat >> "$project_dir"/android/settings.gradle <<EOT
include ':ReactAndroid'
project(':ReactAndroid').projectDir = new File(
    rootProject.projectDir, '../node_modules/react-native/ReactAndroid')
EOT
  sed_op 's/implementation "com.facebook.react:react-native:.*/implementation project(":ReactAndroid")/' "$project_dir"/android/app/build.gradle
fi
commit "Run RN from source"


# Modify app to print a log message at start-up
echo "console.log('Using Hermes: ' + (global.HermesInternal != null));" >> "$project_dir"/index.js
commit "Add smoke test message"

# Build release app
( cd "$project_dir"/android/ && ./gradlew packageRelease )

# Wait for emulator to boot
adb wait-for-device

# Install onto emulator
( cd "$project_dir"/android/ && ./gradlew installRelease )

# Stash the APK
cp "$project_dir"/android/app/build/outputs/apk/release/app-release.apk /tmp/hermes/output/app-release.apk

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
adb shell am start com."$project_dir"/.MainActivity
SECONDS=0
until grep 'Using Hermes: true' <(adb logcat -d -s ReactNativeJS:I) ; do
  if ((SECONDS > 30 )) ; then
    echo "Did not see correct log message within 30s"
    adb logcat -d > /tmp/hermes/output/adblog.txt
    tar cfz /tmp/hermes/output/project.tar.gz "$project_path"
    exit 1
  fi
  sleep 2
done

echo "Test passed: Hermes in use in React Native Awesome App"
exit 0
