#!/bin/bash
cm_lists=$(mktemp /tmp/cmake_check.XXXXXX)
trap 'rm "$cm_lists"' EXIT
find hermes -name CMakeLists.txt -exec cat {} + > "$cm_lists"
for path in $(find hermes -name "*.cpp")
do
    # Except some paths
    case "$path" in
        hermes/facebook/*) continue;;
        hermes/first-party/*) continue;;
    esac
    base=$(basename "$path")
    if ! grep -Fq "$base" "$cm_lists"
    then
        echo "missing $path"
        exit 1
    fi
done
