/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

plugins {
  id("maven-publish")
  id("signing")
}

val signingKey: String? = findProperty("SIGNING_KEY")?.toString()
val signingPwd: String? = findProperty("SIGNING_PWD")?.toString()

val mavenTempLocalUrl = "file:///tmp/maven-local"

publishing {
  publications {
    create<MavenPublication>("release") {
      afterEvaluate {
        println("Available components: ${components.names}")
        from(components["default"])

        version = project.version.toString()
        groupId = project.group.toString()
        artifactId = "hermes-android"
      }

      pom {
        name.set("hermes")
        description.set("A JavaScript engine optimized for fast start-up of React Native apps")
        url.set("https://github.com/facebook/hermes")

        developers {
          developer {
            id.set("facebook")
            name.set("Facebook")
          }
        }

        licenses {
          license {
            name.set("MIT License")
            url.set("https://github.com/facebook/hermes/blob/HEAD/LICENSE")
            distribution.set("repo")
          }
        }

        scm {
          url.set("https://github.com/facebook/hermes.git")
          connection.set("scm:git:https://github.com/facebook/hermes.git")
          developerConnection.set("scm:git:git@github.com:facebook/hermes.git")
        }
      }
    }
  }

  repositories {
    maven {
      name = "mavenTempLocal"
      url = uri(mavenTempLocalUrl)
    }
  }
}

if (signingKey != null && signingPwd != null) {
  logger.info("PGP Key found - Signing enabled")
  signing {
    useInMemoryPgpKeys(signingKey, signingPwd)
    sign(publishing.publications["release"])
  }
} else {
  logger.info("Signing disabled as the PGP key was not found")
}
