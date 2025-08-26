/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import java.io.File
import org.gradle.api.InvalidUserDataException
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.artifacts.VersionCatalogsExtension

class HermesUtilsPlugin : Plugin<Project> {
  override fun apply(project: Project) {
    val hermesUtils = HermesUtils(project)

    project.file(hermesUtils.outputDir).mkdirs()
    project.file("${hermesUtils.outputDir}/aar").mkdirs()

    project.extensions.extraProperties["hermesUtils"] = hermesUtils
  }
}

class HermesUtils(private val project: Project) {
  private val libs =
      project.extensions.getByType(VersionCatalogsExtension::class.java).named("libs")

  val hermesWs: String
    get() {
      val value = System.getenv("HERMES_WS_DIR")
      if (value == null || value == "") {
        throw InvalidUserDataException("HERMES_WS_DIR is not set")
      }
      return value
    }

  val hermesC: String
    get() {
      val candidateHermesCPaths =
          listOf(
              "$hermesWs/build/ImportHostCompilers.cmake",
              "$hermesWs/build_release/ImportHostCompilers.cmake",
          )

      val hermesCPath = candidateHermesCPaths.lastOrNull { File(it).exists() }
      if (hermesCPath == null) {
        project.logger.warn("Could not find hermesC path. Using default path.")
        return "$hermesWs/build/ImportHermesc.cmake"
      }

      return hermesCPath
    }

  // For Facebook internal use:
  val fbsource: String
    get() = System.getenv("FBSOURCE_DIR") ?: "${System.getenv("HOME")}/fbsource"

  val outputDir: String
    get() = "${hermesWs}/build_android/outputs"

  val compileSdk: Int
    get() = libs.findVersion("compileSdk").get().getRequiredVersion().toInt()

  val minSdk: Int
    get() = libs.findVersion("minSdk").get().getRequiredVersion().toInt()

  val facebookBuild: String
    get() = System.getenv("FACEBOOK") ?: "0"

  val cmakeVersion: String
    get() = libs.findVersion("cmake").get().getDisplayName()

  val ndkVersion: String
    get() = libs.findVersion("ndkVersion").get().getDisplayName()

  val abis: List<String>
    get() =
        (project.findProperty("abis") as? String)?.split(",")
            ?: listOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
}
