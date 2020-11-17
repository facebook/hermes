#!/bin/bash -xe
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Wait for emulator to boot
adb wait-for-device

# Print the devices and exit
adb devices

echo "Test passed: Hermes Intl is ready to run !"
exit 0
