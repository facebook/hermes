# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

module HermesHelper
  # BUILD_TYPE = :debug
  BUILD_TYPE = :release

  OSX_DEPLOYMENT_TARGET = "10.13"
  IOS_DEPLOYMENT_TARGET = "10.0"
end

Pod::Spec.new do |spec|
  spec.name        = "hermes"
  spec.version     = "0.7.0"
  spec.summary     = "Hermes is a small and lightweight JavaScript engine optimized for running React Native."
  spec.description = "Hermes is a JavaScript engine optimized for fast start-up of React Native apps. It features ahead-of-time static optimization and compact bytecode."
  spec.homepage    = "https://hermesengine.dev"
  spec.license     = { type: "MIT", file: "LICENSE" }
  spec.author      = "Facebook"
  spec.source      = { git: "https://github.com/facebook/hermes.git", tag: "v#{spec.version}" }
  spec.platforms   = { :osx => HermesHelper::OSX_DEPLOYMENT_TARGET, :ios => HermesHelper::IOS_DEPLOYMENT_TARGET }

  spec.preserve_paths      = ["destroot/bin/*"].concat(HermesHelper::BUILD_TYPE == :debug ? ["**/*.{h,c,cpp}"] : [])
  spec.source_files        = "destroot/include/**/*.h"
  spec.header_mappings_dir = "destroot/include"

  spec.ios.vendored_frameworks = "destroot/Library/Frameworks/iphoneos/hermes.framework"
  spec.osx.vendored_frameworks = "destroot/Library/Frameworks/macosx/hermes.framework"

  spec.xcconfig            = { "CLANG_CXX_LANGUAGE_STANDARD" => "c++14", "CLANG_CXX_LIBRARY" => "compiler-default", "GCC_PREPROCESSOR_DEFINITIONS" => "HERMES_ENABLE_DEBUGGER=1" }

  spec.prepare_command = <<-EOS
    # When true, debug build will be used.
    # See `build-apple-framework.sh` for details
    DEBUG=#{HermesHelper::BUILD_TYPE == :debug}

    # Source utilities into the scope
    . ./utils/build-apple-framework.sh

    # If universal framework for iOS does not exist, build one
    if [ ! -d destroot/Library/Frameworks/iphoneos/hermes.framework ]; then
      build_apple_framework "iphoneos" "armv7;armv7s;arm64" "#{HermesHelper::IOS_DEPLOYMENT_TARGET}"
      build_apple_framework "iphonesimulator" "x86_64;i386" "#{HermesHelper::IOS_DEPLOYMENT_TARGET}"

      create_universal_framework "iphoneos" "iphonesimulator"
    fi

    # If MacOS framework does not exist, build one
    if [ ! -d destroot/Library/Frameworks/macosx/hermes.framework ]; then
      build_apple_framework "macosx" "x86_64;arm64" "#{HermesHelper::OSX_DEPLOYMENT_TARGET}"
    fi
  EOS
end
