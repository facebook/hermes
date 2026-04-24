Downloaded dragonbox.h from https://github.com/jk-jeon/dragonbox/blob/master/include/dragonbox/dragonbox.h, and placed under external/dragonbox/dragonbox/
Downloaded 2025-12-26
Git commit: beeeef91cf6fef89a4d4ba5e95d47ca64ccb3a44

## Patches

Worked around an MSVC VS2019 (`vs16.11.31`) bug with template template
parameter forwarding in a class template's default non-type argument. The
upstream SFINAE pattern `bool = is_in_range<Info, ...>(0)` as a default
template argument of `compute_impl` triggers MSVC errors C2672/C3207 because
the compiler fails to resolve `Info` as a class template in that context.
Wrapped the call in a helper struct `is_in_range_v` and changed the default
to `is_in_range_v<...>::value`; MSVC handles the struct instantiation
correctly. Clang and GCC compile both forms.
Patch: `patches/msvc-is-in-range-wrapper.patch`.
