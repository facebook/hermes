#!/bin/bash
set -e -o pipefail

hbcattribute() {
  "$HBC_ATTRIBUTE"  "$@"
}

symbolicate() {
  "$SYMBOLICATE" "$@"
}

accumulate() {
  "$ACCUMULATE"
}

# Buck passes in a series of variables like 'HBC_ATTRIBUTE_RUN=somevalue'
while [[ $# != 0 && $1 != -- ]]
do
  declare "$1"
  shift
done
shift


case $# in
  1) hbcattribute "$1" | accumulate ;;
  2) hbcattribute "$1" | symbolicate "$2" --attribution | accumulate ;;
  *)
    echo "Usage: $(basename "$0") bundle.hbc [sourcemap.map]" >&2
    exit 1
esac
