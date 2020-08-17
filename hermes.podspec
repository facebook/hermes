# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

module HermesHelper
  # BUILD_TYPE = :debug
  BUILD_TYPE = :release
end

Pod::Spec.new do |spec|
  spec.name        = "hermes"
  spec.version     = "0.5.0"
  spec.summary     = "Hermes is a small and lightweight JavaScript engine optimized for running React Native."
  spec.description = "Hermes is a JavaScript engine optimized for fast start-up of React Native apps. It features ahead-of-time static optimization and compact bytecode."
  spec.homepage    = "https://hermesengine.dev"
  spec.license     = { type: "MIT", file: "LICENSE" }
  spec.author      = "Facebook"
  spec.source      = { git: "https://github.com/facebook/hermes.git", tag: "v#{spec.version}" }
  spec.platforms   = { :osx => "10.14", :ios => "10.0" }

  spec.preserve_paths      = ["destroot/bin/*"].concat(HermesHelper::BUILD_TYPE == :debug ? ["**/*.{h,c,cpp}"] : [])
  spec.source_files        = "destroot/include/**/*.h"
  spec.header_mappings_dir = "destroot/include"
  spec.vendored_frameworks = "destroot/Library/Frameworks/hermes.framework"
  spec.xcconfig            = { "CLANG_CXX_LANGUAGE_STANDARD" => "c++14", "CLANG_CXX_LIBRARY" => "compiler-default", "GCC_PREPROCESSOR_DEFINITIONS" => "HERMES_ENABLE_DEBUGGER=1" }

  spec.script_phase = { :name => 'Build source files', :script => File.read("./configure-ios.sh")}
  
end
