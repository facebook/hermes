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
cd $HERMES_ROOT_LOCATION
hermes/utils/build/configure.py --cmake-flags='-DHERMES_ENABLE_FUZZILLI=ON -DHERMES_ENABLE_TRACE_PC_GUARD=ON'
cd build
ninja
```

Start Fuzzing:
```shell
cd $FUZZILLI_LOCATION
swift run  FuzzilliCli --profile=hermes $BINARY_LOCATION
```
