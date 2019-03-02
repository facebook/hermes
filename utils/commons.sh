#!/usr/bin/env bash

if ! (return 0 2>/dev/null); then
  echo "commons.sh must be sourced"
  exit 1
fi

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
