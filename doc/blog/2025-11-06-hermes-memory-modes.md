# Hermes Memory Modes

*November 6, 2025 · tmikov*

Hermes has the following memory modes:

## HV64 (all platforms)

This is usually the simplest mode - all JS values, pointers, and numbers are encoded as 64-bit values using NaN-boxing.

## HV32 on 32-bit devices

JS values and pointers are encoded as 32-bit values. Some numbers are encoded as 32-bit values. The ones that do not fit in 32-bit are boxed in the heap. Fortunately, in most apps this is a really small percentage.

This mode has a small overhead, but it saves a lot of memory, obviously, and can be a performance win because of smaller cache footprint.

## HV32 for 64-bit platforms (except iOS)

JS values and pointers are encoded as 32-bit values. All memory is allocated in a (up to) 4GB reserved contiguous memory block and all pointers are 32-bit offsets inside it.

This mode similarly has some overhead but saves a lot of memory, and can be a performance win because of smaller cache footprint.

## HV32 for iOS (64-bit)

iOS is special, since it doesn't by default allow an app to allocate a lot of virtual address space (without a special entitlement). So, Hermes allocates memory in 4MB segments. Each segment can be anywhere in the 64-bit address space, so there is no way to directly encode a 64-bit address in a 32-bit pointer.

So, Hermes maintains a table with one entry per segment. 32-bit pointers are a combination of index in the table and offset in the segment (I am simplifying a bit, but the principle remains).

This mode also saves a lot of memory like the other HV32 ones, but has more overhead. iOS devices usually are faster, so it is not as visible.

---

As you can see, Hermes is very configurable. A lot of work has gone into supporting different kinds of systems.

With Hermes V1, by default we are going to ship HV32 on Android devices and HV64 on iOS devices in order to get best performance from the latter.
