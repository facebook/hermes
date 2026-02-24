#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

[ -z "$juno" ] && echo "juno not specified" && exit 1

cp ../main.mjs .

for f in ../*.js; do
    nf=${f#*/}
    nf=${nf%.js}
    echo "$f"
    "$juno" --dialect=flow --strip-flow --gen-js "$f" | prettier --no-config --parser=babel > "${nf}.js"
done

# Delete extra copy of main.mjs used by Webpack.
rm main.js
