#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if ! (return 0 2>/dev/null); then
  echo "commons.sh must be sourced"
  exit 1
fi

# Run a command repeatedly until it success (exit code is 0), up to X times.
# For example, "retry 3 make -j2" will run "make -j2" repeatedly until
# it fails 3 times or succeeds.
function retry() {
  RETRY=$1
  ATTEMPTS=0
  while (( ATTEMPTS <= RETRY )); do
    "${@:2}" && return 0
    ATTEMPTS=$((ATTEMPTS+1))
    echo "Retry: $ATTEMPTS failures so far"
  done
  echo "Retry depleted for command: ${*:2}"
  return 1
}

# Determine platform so that things like NT
# don't need to be littered everywhere.
case "$(uname)" in
  "Linux")
    PLATFORM="linux" ;;
  "Darwin")
    PLATFORM="macosx" ;;
  *"_NT-"*)
    PLATFORM="windows" ;;
  *)
    PLATFORM="unknown" ;;
esac

# Setup Windows specific path conversion
if [[ "$PLATFORM" == "windows" ]]; then
  platform_path() {
    # Cygwin, MSYS, and MSYS2 all provides cygpath
    cygpath -m -- "$@"
  }
  # Disable MSYS and MSYS2 automatic path conversion. This script needs to run
  # in both Cygwin and MSYS*. Allowing the automatic path conversion,
  # which isn't available in Cygwin, allows bugs to hide.
  export MSYS2_ARG_CONV_EXCL="*"
  export MSYS_NO_PATHCONV=1
else
  platform_path() {
    echo "$@"
  }
fi
