# Hermes JS VM

**Welcome to Hermes!**

Hermes is a JavaScript virtual machine that accepts [Flow](https://flowtype.org)
type annotations, features host-side static optimizations and compact bytecode.

## Getting Started

### System Requirements

The project builds and runs on OS X and Linux. The project depends on the tools
that are required to build LLVM (a C++ compiler, CMake, Python, etc) as well as
node.js and babel.

The REPL uses 'libreadline' for editing, if it is installed.

### Getting Sources and building the compiler

Running the LLVM build script will clone LLVM and Clang (which are a dependency)
and setup the symlinks that LLVM needs to build clang. Next, the script will
build llvm and cland intto the directory "llvm_build".

    ./hermes/utils/build_llvm.sh

Next, run the configure script. The script will generate a build directory with
Ninja project files. It is possible to configure and build Hermes with any CMake
generator, like GNU Makefiles and Xcode build. Peek into the configure script
if you prefer to use an alternative build system.

    ./hermes/utils/configure.sh

After running the build script, running 'ninja' from the build directory will
build the compiler driver.

## Testing and running Hermes

After compiling the project, the vm driver binary will be located in the '/bin'
directory under the name './bin/hermes'.  Run './bin/hermes --help' to learn
more about using the vm test driver.

To run the tests run the 'check-hermes' target. If you are using the default
build system, ninja, then the command to run the tests is 'ninja check-hermes'.

The default compilation mode is 'Debug'. This means that the compiler itself is
easy to debug because it has debug info, lots of assertions, and the
optimizations are disabled. If you wish to benchmark the compiler or release it
then you should compile the compiler in Release mode.

When configuring the project add the flag "-DCMAKE_BUILD_TYPE=Release". Refer to
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

