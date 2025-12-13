# JSON Parsing Performance: 2.7x-3.4x Faster

*November 24, 2025 · tmikov*

Big news for Hermes JSON parsing. Michael from our team worked on speeding it up for the last month - the result is between 2.7x-3.4x faster! It is available in the static_h branch now and will be included in the next stable releases.

Incomplete list of improvements:

- Switch to an iterative parser
- Cache object hidden classes
- Optimize string parsing
- Use [@lemire](https://x.com/lemire)'s [fast_float](https://github.com/fastfloat/fast_float)
- Add an even faster number path for integers
- Avoid unnecessary allocations
- Use a per-character lookup and function dispatch table

Coming up next: JSON.stringify().
