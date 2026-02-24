# ISerialization: Efficient Binary Encoding for JS Values

*December 2, 2025 · tmikov*

ISerialization just landed in the static_h branch and will be out in the next stable release.

ISerialization encodes a JS value into an efficient opaque binary representation, which can then be de-serialized (deeply cloned) in one or more runtime instances.

It also optionally supports serialization with ownership transfer.

It is intended for efficient passing of values between runtimes and will be the base of message passing in our implementation of Web Workers.
