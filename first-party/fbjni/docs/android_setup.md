## Android Build Setup

**These instructions require the Android Gradle Build Plugin 4.0.0 or newer
as it relies on the new [prefab](https://android-developers.googleblog.com/2020/02/native-dependencies-in-android-studio-40.html) integration.
Check below for a pre-4.0.0 workaround.**

```groovy
repositories {
  maven {
    mavenCentral()
  }
}

android {
  dependencies {
    implementation 'com.facebook.fbjni:fbjni:0.2.2'
  }
}
```

Now, in your CMake setup, you can refer to the `fbjni` package. The header files (e.g. `fbjni.h`)
will available implicitly.

```cmake
set(build_DIR ${CMAKE_SOURCE_DIR}/build)
set(PACKAGE_NAME "myapp")

find_package(fbjni REQUIRED CONFIG)

target_link_libraries(${PACKAGE_NAME} fbjni::fbjni mylibrary)
```

## Android Build Setup (pre-4.0.0)

The Android Gradle plugin does not provide built-in support for artifacts that
include native libraries (for linking against) and header files. Because of
that, some manual additions to your build system are required. The following
is an example of what this can look like but by no means prescriptive.

In your app-specific `build.gradle`:

```groovy
repositories {
  maven {
    jcenter()
  }
}

android {
  // Create new configurations that can be referred to in dependencies.
  // The Android Gradle Plugin 3.* does not allow hooking into existing
  // configurations like `implementation`.
  configurations {
    extractHeaders
    extractJNI
  }

  dependencies {
    implementation 'com.facebook.fbjni:fbjni:0.0.1'
    // If headers are required.
    extractHeaders 'com.facebook.fbjni:fbjni:0.0.1:headers'
    // If the `.so` files are required for linking.
    extractJNI 'com.facebook.fbjni:fbjni:0.0.1'
  }
}

task extractAARHeaders {
    doLast {
        configurations.extractHeaders.files.each {
            def file = it.absoluteFile
            copy {
                from zipTree(file)
                into "$buildDir/$file.name"
                include "**/*.h"
            }
        }
    }
}

task extractJNIFiles {
    doLast {
        configurations.extractJNI.files.each {
            def file = it.absoluteFile
            copy {
                from zipTree(file)
                into "$buildDir/$file.name"
                include "jni/**/*"
            }
        }
    }
}

tasks.whenTaskAdded { task ->
    if (task.name.contains('externalNativeBuild')) {
        task.dependsOn(extractAARHeaders)
        task.dependsOn(extractJNIFiles)
    }
}
```

With this setup in place, prior to any native build jobs, the header files
and JNI shared libraries will be extracted under the build directory.

Now, in your CMake setup, you can refer to the extracted paths:

```cmake
set(build_DIR ${CMAKE_SOURCE_DIR}/build)
set(PACKAGE_NAME "myapp")

file(GLOB libfbjni_link_DIRS "${build_DIR}/fbjni*.aar/jni/${ANDROID_ABI}")
file(GLOB libfbjni_include_DIRS "${build_DIR}/fbjni-*-headers.jar/")

find_library(FBJNI_LIBRARY fbjni PATHS ${libfbjni_link_DIRS}
NO_CMAKE_FIND_ROOT_PATH)

target_include_directories(${PACKAGE_NAME} PRIVATE
  // Additional header directories here
  ${libfbjni_include_DIRS}
)
target_link_libraries(${PACKAGE_NAME} ${FBJNI_LIBRARY} mylibrary)
```
