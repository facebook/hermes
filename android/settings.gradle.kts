/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

pluginManagement {
  repositories {
    google()
    mavenCentral()
  }
}

rootProject.name = "Hermes"

include(":hermes")
include(":cppruntime")
include(":intltest")
