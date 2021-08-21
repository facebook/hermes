# Hermes Source Map

This is a simple wrapper around the Hermes Source Map C API.

## Bindgen FFI

The wrapper uses bindgen to access the C API. The FFI is already pre-generated, but if it needs to be updated, from the current directory run:
```shell
./bindgen.sh
```

Bindgen seems to really be meant to be run at build time, since it generates binding using raw types used by the current platform instead of platform-independent types exported by the `libc` crate. We don't think the associated complexity is justified for a simple API like this, so we have taken care to make the generated FFI portable by stripping uneeded types and injecting `libc::size_t`. This approach may not work for more complex APIs. 
