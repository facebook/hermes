/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

plugins { `kotlin-dsl` }

repositories {
  mavenCentral()
  gradlePluginPortal()
}

group = "com.facebook.hermes"

kotlin {
  @Suppress("MagicNumber") jvmToolchain(17)
  compilerOptions {
    allWarningsAsErrors = providers.gradleProperty("warningsAsErrors").orNull.toBoolean()
  }
}
