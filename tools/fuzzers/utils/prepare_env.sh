# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Install libprotobuf-mutator
if [ ! -d "LPM" ]; then
  rm -rf LPM libprotobuf-mutator
  git clone --depth 1 https://github.com/google/libprotobuf-mutator.git
  mkdir LPM && cd LPM && cmake ../libprotobuf-mutator \
    -GNinja -DLIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF=ON \
    -DLIB_PROTO_MUTATOR_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release && ninja && ninja install
else
  echo "INFO: LPM already exists, skipping libprotobuf-mutator build"
fi