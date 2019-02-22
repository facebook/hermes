#!/bin/bash
set -e
trap 'echo "FAILED"' ERR

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
  # Cherry pick required JSI changes
  git cherry-pick ac90c4fd6d5 7a8957c0bea03 0d7faf6f73b
  git apply ../../../hermes/first-party/patches/include-hermes-executor.diff
  yarn install
)

green "Done"
