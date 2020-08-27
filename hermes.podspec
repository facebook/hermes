# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

module HermesHelper
  # BUILD_TYPE = :debug
  BUILD_TYPE = :release

  DEPLOYMENT_TARGET = "10.13"

  def self.command_exists?(bin)
    "command -v #{bin} > /dev/null 2>&1"
  end

  def self.cmake_configuration
    "-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING='#{DEPLOYMENT_TARGET}' -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true -DHERMES_ENABLE_FUZZING:BOOLEAN=false -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true -DHERMES_BUILD_APPLE_DSYM:BOOLEAN=true"
  end

  def self.configure_command
    "./utils/build/configure.py #{BUILD_TYPE == :release ? "--distribute" : "--build-type=Debug"} --cmake-flags='#{HermesHelper.cmake_configuration} -DCMAKE_INSTALL_PREFIX:PATH=../destroot' build"
  end
end

Pod::Spec.new do |spec|
  spec.name        = "hermes"
  spec.version     = "0.6.0"
  spec.summary     = "Hermes is a small and lightweight JavaScript engine optimized for running React Native."
  spec.description = "Hermes is a JavaScript engine optimized for fast start-up of React Native apps. It features ahead-of-time static optimization and compact bytecode."
  spec.homepage    = "https://hermesengine.dev"
  spec.license     = { type: "MIT", file: "LICENSE" }
  spec.author      = "Facebook"
  spec.source      = { git: "https://github.com/facebook/hermes.git", tag: "v#{spec.version}" }
  spec.platforms   = { :osx => HermesHelper::DEPLOYMENT_TARGET }

  spec.preserve_paths      = ["destroot/bin/*"].concat(HermesHelper::BUILD_TYPE == :debug ? ["**/*.{h,c,cpp}"] : [])
  spec.source_files        = "destroot/include/**/*.h"
  spec.header_mappings_dir = "destroot/include"
  spec.vendored_frameworks = "destroot/Library/Frameworks/hermes.framework"
  spec.xcconfig            = { "CLANG_CXX_LANGUAGE_STANDARD" => "c++14", "CLANG_CXX_LIBRARY" => "compiler-default", "GCC_PREPROCESSOR_DEFINITIONS" => "HERMES_ENABLE_DEBUGGER=1" }

  spec.prepare_command = <<-EOS
    if [ ! -d destroot/Library/Frameworks/hermes.framework ]; then
      if #{HermesHelper.command_exists?("cmake")}; then
        if #{HermesHelper.command_exists?("ninja")}; then
          #{HermesHelper.configure_command} --build-system='Ninja' && cd build && ninja install/strip
        else
          #{HermesHelper.configure_command} --build-system='Unix Makefiles' && cd build && make install/strip
        fi
      else
        echo >&2 'CMake is required to install Hermes, install it with: brew install cmake'
        exit 1
      fi
    fi
  EOS
end
