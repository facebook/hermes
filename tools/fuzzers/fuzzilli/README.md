# Fuzzing with Fuzzilli

Get Fuzzilli
```shell
git clone https://github.com/googleprojectzero/fuzzilli.git
```

Setup Profiles for Hermes
```shell
mv profile/*.swift $FUZZILLI_LOCATION/Sources/FuzzilliCli/Profiles/
```

Build Instrumented Binary
```shell
mkdir fuzzilli_build && cd fuzzilli_build
cmake .. -DHERMES_ENABLE_FUZZILLI=ON -DHERMES_ENABLE_TRACE_PC_GUARD=ON $OTHER_FLAGS
make fuzzilli
```
If you are using the recent hermes source code, you need to comment out the following code in order to compile successfully.
external/llvh/cmake/config-ix.cmake:323
```

# Define LLVM_HAS_ATOMICS if gcc or MSVC atomic builtins are supported.
# include(CheckAtomic)
```
Start Fuzzing:
```shell
cd $FUZZILLI_LOCATION
swift run  FuzzilliCli --profile=hermes $BINARY_LOCATION
```
