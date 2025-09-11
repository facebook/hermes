/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

plugins {
  id("maven-publish")
  id("publish")
}

group = "com.facebook.hermes-v2"

version = project.findProperty("VERSION_NAME")?.toString()!!

configurations.maybeCreate("iosArtifacts")

// Those artifacts should be placed inside the `artifacts/hermes-ios-*.tar.gz` location.
val hermesiOSDebugArtifactFile: RegularFile =
    layout.projectDirectory.file("artifacts/hermes-ios-debug.tar.gz")
val hermesiOSDebugArtifact: PublishArtifact =
    artifacts.add("iosArtifacts", hermesiOSDebugArtifactFile) {
      type = "tgz"
      extension = "tar.gz"
      classifier = "hermes-ios-debug"
    }
val hermesiOSReleaseArtifactFile: RegularFile =
    layout.projectDirectory.file("artifacts/hermes-ios-release.tar.gz")
val hermesiOSReleaseArtifact: PublishArtifact =
    artifacts.add("iosArtifacts", hermesiOSReleaseArtifactFile) {
      type = "tgz"
      extension = "tar.gz"
      classifier = "hermes-ios-release"
    }

// Those artifacts should be placed inside the `artifacts/hermes-*.framework.dSYM` location
val hermesDSYMDebugArtifactFile: RegularFile =
    layout.projectDirectory.file("artifacts/hermes-framework-dSYM-debug.tar.gz")
val hermesDSYMDebugArtifact: PublishArtifact =
    artifacts.add("iosArtifacts", hermesDSYMDebugArtifactFile) {
      type = "tgz"
      extension = "tar.gz"
      classifier = "hermes-framework-dSYM-debug"
    }
val hermesDSYMReleaseArtifactFile: RegularFile =
    layout.projectDirectory.file("artifacts/hermes-framework-dSYM-release.tar.gz")
val hermesDSYMReleaseArtifact: PublishArtifact =
    artifacts.add("iosArtifacts", hermesDSYMReleaseArtifactFile) {
      type = "tgz"
      extension = "tar.gz"
      classifier = "hermes-framework-dSYM-release"
    }

publishing {
  publications {
    getByName("release", MavenPublication::class) {
      artifactId = "hermes-ios"
      artifact(hermesiOSDebugArtifact)
      artifact(hermesiOSReleaseArtifact)
      artifact(hermesDSYMDebugArtifact)
      artifact(hermesDSYMReleaseArtifact)
    }
  }
}
