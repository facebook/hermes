# NPM instructions

## When building artefacts locally

```bash
./src/utils/build/build_llvm.py --distribute --cmake-flags='-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.13 -DCMAKE_OSX_ARCHITECTURES:STRING=x86_64;arm64'
./src/utils/build/configure.py --distribute --cmake-flags='-DHERMES_ENABLE_DEBUGGER:BOOLEAN=true -DHERMES_ENABLE_FUZZING:BOOLEAN=false -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true -DHERMES_BUILD_APPLE_DSYM:BOOLEAN=true -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.13 -DCMAKE_OSX_ARCHITECTURES:STRING=x86_64;arm64 -DCMAKE_INSTALL_PREFIX:PATH=../destroot' build
cd build_release
ninja hermes-runtime-darwin-cocoapods-release
mv github/hermes-runtime-darwin-v*.tar.gz ../src/npm/hermes-engine-darwin/
```

## Or when fetching artefacts from CI

Get the URL of the `hermes-runtime-darwin-v*.tar.gz` tarball from the `build-macos-runtime` CI job.

```bash
curl -L -O [ARTEFACT_URL]
mv hermes-runtime-darwin-v*.tar.gz /path/to/src/npm/hermes-engine-darwin/
```

## Pack package

```bash
cd npm/hermes-engine-darwin
yarn pack
```
