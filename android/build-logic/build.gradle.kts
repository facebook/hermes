/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

plugins {
  `kotlin-dsl`
  kotlin("jvm") version "2.1.20" // or whatever version you're using
}

repositories {
  mavenCentral()
  gradlePluginPortal()
}

group = "com.facebook.hermes"

kotlin {
  @Suppress("MagicNumber") jvmToolchain(17)
}

gradlePlugin {
  plugins {
    register("testUtils") {
      id = "com.facebook.hermes.plugins.internal.testUtils"
      implementationClass = "TestUtilsPlugin"
    }
  }
}
