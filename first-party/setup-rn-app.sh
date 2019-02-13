#!/bin/bash
set -e

die() {
  echo "$*"
  exit 1
}
green() {
  echo "$(tput setaf 2)$*$(tput sgr0)"
}
[ -f App.js ] || die "Please run me from a newly 'react-native init'd app"
[ -d ../hermes ] || die "Expected hermes at ../hermes"

green "Running initial app setup"
( cd android && ./gradlew assemble )

green "Patching app to use RN from source"
patch -p 1 < ../hermes/first-party/patches/react-from-source.diff

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
  git checkout -b stable 1024dc251e1f4
  git apply ../../../hermes/first-party/patches/include-hermes-executor.diff
  yarn install
)

green "Done"
