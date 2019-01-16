#!/bin/bash
cm_lists=$(mktemp /tmp/cmake_check.XXXXXX)
trap 'rm "$cm_lists"' EXIT
find hermes -name CMakeLists.txt -exec cat {} + > "$cm_lists"
for path in $(find hermes -name "*.cpp")
do
    base=$(basename "$path")
    if ! grep -Fq "$base" "$cm_lists"
    then
        echo "missing $base"
        exit 1
    fi
done
