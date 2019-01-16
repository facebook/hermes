The Public directory contains type declarations made available to both the
Hermes runtime and the Hermes API. The idea is to give both projects a shared
vocabulary of types to reduce the need for bridging and glue code.

The Public directory may not use types from the Hermes, VM, or API layers, or
LLVM types. It may use std types and its own types only.
