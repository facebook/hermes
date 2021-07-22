# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ -z $LPM_LOC ]; then
  LPM_LOC="../utils/LPM";
fi


if [ -d $LPM_LOC ]; then
  echo "Attempting to compile json.proto with LPM's protoc";
  ../utils/LPM/external.protobuf/bin/protoc ./json.proto --cpp_out=.
  echo "Done!"
else
  echo "LPM not installed, run ../utils/prepare_env.sh to install it";
fi