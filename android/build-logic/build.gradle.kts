/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

plugins {
  id("java-gradle-plugin")
  alias(libs.plugins.kotlin.jvm)
  alias(libs.plugins.ktfmt)
}

repositories { mavenCentral() }

dependencies { implementation("org.apache.commons:commons-compress:1.27.1") }

gradlePlugin {
  plugins.register("buildlogic") {
    id = "buildlogic"
    implementationClass = "BuildPlugin"
  }
}
