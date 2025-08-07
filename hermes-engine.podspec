# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

Pod::Spec.new do |spec|
  spec.name        = "hermes-engine"
  spec.version     = "0.12.0"
  spec.summary     = "Hermes is a small and lightweight JavaScript engine optimized for running React Native."
  spec.description = "Hermes is a JavaScript engine optimized for fast start-up of React Native apps. It features ahead-of-time static optimization and compact bytecode."
  spec.homepage    = "https://hermesengine.dev"
  spec.license     = { type: "MIT", file: "LICENSE" }
  spec.author      = "Facebook"
  # This env var should be supplied with a CDN URL of the "hermes-runtime-darwin.tgz" on the Github releases before pod push.
  # The podspec would be serialized to JSON and people will download prebuilt binaries instead of the source.
  # TODO(use the hash field as a validation mechanism when the process is stable)
  spec.source      = ENV['hermes-artifact-url'] ? { http: ENV['hermes-artifact-url'] } : { git: "https://github.com/facebook/hermes.git", tag: "v#{spec.version}" }
  spec.platforms   = { :osx => "10.13", :ios => "12.0", :visionos => "1.0", :tvos => "12.0" }

  spec.preserve_paths      = '**/*.*'
  spec.source_files        = ''

  spec.pod_target_xcconfig = {
                    "CLANG_CXX_LANGUAGE_STANDARD" => "c++20",
                    "CLANG_CXX_LIBRARY" => "compiler-default",
                    "GCC_PREPROCESSOR_DEFINITIONS[config=Debug]" => "$(inherited) HERMES_ENABLE_DEBUGGER=1",
                    "GCC_PREPROCESSOR_DEFINITIONS[config=Release]" => "$(inherited) HERMES_ENABLE_DEBUGGER=0"
                  }

  spec.ios.vendored_frameworks = "destroot/Library/Frameworks/universal/hermes.xcframework"
  spec.visionos.vendored_frameworks = "destroot/Library/Frameworks/universal/hermes.xcframework"
  spec.tvos.vendored_frameworks = "destroot/Library/Frameworks/universal/hermes.xcframework"
  spec.osx.vendored_frameworks = "destroot/Library/Frameworks/macosx/hermes.framework"

  spec.subspec 'Hermes' do |ss|
    ss.source_files = ''
    ss.public_header_files = 'destroot/include/hermes/*.h'
    ss.header_dir = 'hermes'
  end

  spec.subspec 'cdp' do |ss|
    ss.source_files = ''
    ss.public_header_files = 'destroot/include/hermes/cdp/*.h'
    ss.header_dir = 'hermes/cdp'
  end

  spec.subspec 'inspector' do |ss|
    ss.source_files = ''
    ss.public_header_files = 'destroot/include/hermes/inspector/*.h'
    ss.header_dir = 'hermes/inspector'
  end

  spec.subspec 'inspector_chrome' do |ss|
    ss.source_files = ''
    ss.public_header_files = 'destroot/include/hermes/inspector/chrome/*.h'
    ss.header_dir = 'hermes/inspector/chrome'
  end

  spec.subspec 'jsi' do |ss|
    ss.source_files = ''
    ss.public_header_files = 'destroot/include/jsi/*.h'
    ss.header_dir = 'jsi'
  end

  spec.subspec 'Public' do |ss|
    ss.source_files = ''
    ss.public_header_files = 'public/hermes/Public/*.h'
    ss.header_dir = 'hermes/Public'
  end

  hermesc_path = "${PODS_ROOT}/hermes-engine/build_host_hermesc"

  if ENV.has_key?('HERMES_OVERRIDE_HERMESC_PATH') && File.exist?(ENV['HERMES_OVERRIDE_HERMESC_PATH']) then
    hermesc_path = ENV['HERMES_OVERRIDE_HERMESC_PATH']
  end

  spec.user_target_xcconfig = {
    'HERMES_CLI_PATH' => "#{hermesc_path}/bin/hermesc"
  }
end
