# hermes_estree

This crate is a Rust representation of the [ESTree format](https://github.com/estree/estree/tree/master) and
popular extenions including JSX and (eventually) Flow and TypeScript.

Because this crate is intended to support usage in both Rust toolchains and for interop in JavaSCript-based toolchains, the format is designed to support accurate serialization to/from estree-compatible JSON.
