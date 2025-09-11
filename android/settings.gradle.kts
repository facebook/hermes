/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

pluginManagement {
  includeBuild("build-logic")

  repositories {
    mavenCentral()
    google()
    gradlePluginPortal()
  }
}

rootProject.name = "hermes-engine"

include(":cppruntime")

include(":intltest")
