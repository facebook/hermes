/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.helpers.internal

import groovy.json.JsonSlurper
import java.io.File
import org.gradle.api.Project

data class PackageJson(val version: String)

class SerializeHermesCompilerPackageJson(project: Project) {
  private val serializedPackageJson = run {
    val packageJsonFile = File(project.rootDir.parentFile, "npm/hermes-compiler/package.json")
    if (!packageJsonFile.exists()) {
      throw IllegalStateException("package.json file not found at $packageJsonFile")
    }

    val jsonSlurper = JsonSlurper().parseText(packageJsonFile.readText()) as Map<String, Any>
    val version =
        jsonSlurper["version"] as? String
            ?: throw IllegalStateException(
                "Expected version to be a string in package.json, but got ${jsonSlurper["version"]}"
            )
    PackageJson(version)
  }

  val hermesReleaseVersion = serializedPackageJson.version
}
