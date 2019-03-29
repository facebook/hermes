#!/bin/bash
set -e
trap 'echo "FAILED"' ERR

die() {
  red "$*"
  exit 1
}
green() {
  echo "$(tput setaf 2)$*$(tput sgr0)"
}
red() {
  echo "$(tput setaf 1)$*$(tput sgr0)"
}

[ -f App.js ] || die "Please run me from a newly 'react-native init'd app"
[ -d ../hermes ] || die "Expected hermes at ../hermes"

if [ -d ../hermes/API/jsi/jsi ]
then
  jsi="$PWD/../hermes/API/jsi/jsi"
elif [ -d ../hermes/../jsi/jsi ]
then
  jsi="$PWD/../hermes/../jsi/jsi"
else
  die "Can't find JSI"
fi

green "Running initial app setup"
( cd android && ./gradlew assemble )

green "Patching app to use RN from source"
patch -p 1 < ../hermes/first-party/patches/react-from-source.diff

# RN minor versions change weekly, so just replace whatever it currently is
cp package.json package.json.bak
sed -e 's|^ *"react-native".*|    "react-native": "github:facebook/react-native#master"|' package.json.bak > package.json
rm package.json.bak

green "Patching app to use Hermes as its executor"
(
  # This is slightly awkward due to the project name changing
  patch=$PWD/../hermes/first-party/patches/use-hermes.diff
  cd android/app/src/main/java/com/*
  patch -p 8 < "$patch"
)

green "Checking out RN"

(
  cd node_modules
  rm -r react-native
  git clone https://github.com/facebook/react-native
  cd react-native
  # Check out a revision before 'Could not get unknown property 'mergeResourcesProvider'
  git checkout -b stable 1024dc251e1f4
  # Apply JSI/JSIExecutor refactorings
  git cherry-pick 96de12ab48 81860c59c3 19866aee3d3 64f3a87c9d2e dcc40a6267b4
  # Copy in the latest version of JSI
  cp -f "$jsi"/* ReactCommon/jsi/ && rm -f ReactCommon/jsi/*testlib*
  git commit -a -m "Auto-update JSI"
  # Make the JSCRuntime compatible with updated JSI
  git apply ../../../hermes/first-party/patches/update-jsc-runtime.diff
  git commit -a -m "Auto-patch JSCRuntime"
  # Set up the HermesExecutor
  git apply ../../../hermes/first-party/patches/include-hermes-executor.diff
  git commit -a -m "Auto-add HermesExecutor"
  yarn install
)

green "Done"
