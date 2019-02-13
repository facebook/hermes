# Hermes JS VM

**Welcome to Hermes!**

Hermes is a JavaScript virtual machine that accepts [Flow](https://flowtype.org)
type annotations, features host-side static optimizations and compact bytecode.

## Getting Started for React Native developers

Create a base directory to work in, e.g. `~/workspace`, and `cd` into it. Then follow the steps below (or copy-paste it all):

```
( set -e

# 1. Use this directory as the workspace
export HERMES_WS_DIR="$PWD"

# 2. Clone Hermes here
git clone git:github.com/facebook/hermes

# 3. Clone and build LLVM. This may take a while.
hermes/utils/build_llvm.sh

# 4. Cross-compile LLVM dependencies for all Android ABIs
hermes/utils/crosscompile_llvm.sh

# 5. Compile libhermes for Android
( cd hermes/android && gradle build )

# 6. Create a react-native project
react-native init AwesomeProject

# 7. Patch this project to use libhermes
( cd AwesomeProject && "$HERMES_WS_DIR/hermes/first-party/setup-rn-app.sh" )

)
```

To set up an existing project to use Hermes:

1. Set up the project to use [React Native from source](https://facebook.github.io/react-native/docs/building-from-source).
   Note that their master branch may have moved on. The setup-rn-app script
   uses [commit
   1024dc251](https://github.com/facebook/react-native/commit/1024dc251e1f4777052b7c41807ea314672bb13a).
2. Patch react-native to build the HermesExecutor using
   [include-hermes-executor.diff](first-party/patches/include-hermes-executor.diff).
3. Override `ReactNativeHost.getJavaScriptExecutorFactory()` to return a
   `HermesExecutorFactory` as in
   [use-hermes.diff](first-party/patches/use-hermes.diff).
4. Verify that you are using Hermes by checking that `typeof(HermesInternal)`
   is `object` (and not `undefined`).


## Getting Started for Hermes developers

### System Requirements

The project builds and runs on OS X and Linux. The project depends on the tools
that are required to build LLVM (a C++ compiler, CMake, Python, etc) as well as
node.js and babel.

The REPL uses 'libreadline' for editing, if it is installed.

### Getting Sources and building the compiler

Running the LLVM build script will clone LLVM and Clang (which are a dependency)
and setup the symlinks that LLVM needs to build clang. Next, the script will
build llvm and clang into the directory "llvm_build".

    ./hermes/utils/build_llvm.sh

Next, run the configure script. The script will generate a build directory with
Ninja project files. It is possible to configure and build Hermes with any CMake
generator, like GNU Makefiles and Xcode build. Peek into the configure script
if you prefer to use an alternative build system.

    ./hermes/utils/configure.sh

After running the build script, running 'ninja' from the build directory will
build the compiler driver.

## Testing and running Hermes

After compiling the project, the vm driver binary will be located in the `/bin`
directory under the name `./bin/hermes`.  Run `./bin/hermes --help` to learn
more about using the vm test driver.

To run the tests run the `check-hermes` target. If you are using the default
build system, ninja, then the command to run the tests is `ninja check-hermes`.

The default compilation mode is "Debug". This means that the compiler itself is
easy to debug because it has debug info, lots of assertions, and the
optimizations are disabled. If you wish to benchmark the compiler or release it
then you should compile the compiler in Release mode.

When configuring the project add the flag `-DCMAKE_BUILD_TYPE=Release`. Refer to
the LLVM build instructions for more details:

    http://llvm.org/docs/GettingStarted.html

## Debugging Hermes

One excellent tool for catching memory errors is Clang's Address Sanitizer
(ASan). To enable an ASan build, configure the project with the flag
"-DLLVM_USE_SANITIZER="Address". Another option is to add the following line to
the main hermes CMake file:

  set(LLVM_USE_SANITIZER "Address")

For more details about ASan:

  http://clang.llvm.org/docs/AddressSanitizer.html

## Supported targets

At the moment Mac and Linux and the only supported targets. The vm
should compile and run on Windows, but this configuration has not been tested.

## Contributing to Hermes

Contributions to Hermes are welcomed and encouraged!
