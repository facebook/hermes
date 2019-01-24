#!/bin/bash
cm_lists=$(mktemp /tmp/cmake_check.XXXXXX)
trap 'rm "$cm_lists"' EXIT
find hermes -name CMakeLists.txt -exec cat {} + > "$cm_lists"
for path in $(find hermes -name "*.cpp")
do
    # We exempt the LLVM files.
    if [[ $path =~ ^hermes/facebook/llvm ]]; then
        continue
    fi
    base=$(basename "$path")
    if ! grep -Fq "$base" "$cm_lists"
    then
        echo "missing $path"
        exit 1
    fi
done
