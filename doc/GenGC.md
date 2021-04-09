---
id: gengc
title: The GenGC Garbage Collector
---

GenGC used to be the default garbage collector for Hermes, and aims to provide a
generational garbage collector that returns memory to the OS aggressively and
frequently. The newer GC is called [Hades](./Hades.md), and it has much lower
pause times than GenGC. We recommend most users use Hades instead.

Throughout this document, we will refer to the **heap**, which in this context
means the memory space where JS objects reside. This is separate from the C and
C++ `malloc` heap, and in general if this document says "heap" it means the
space for JS objects.

# Heap Segments

The heap is made out of many fixed-size regions of memory known as
**heap segments** (or more briefly as **segments**). Currently these segments
are 4 MiB, but this can be changed with the CMake build configuration variable
`-DHERMESVM_HEAP_SEGMENT_SIZE_KB=number`. Memory is acquired and released on a
per-segment basis. Segments are allocated using `mmap` on POSIX systems and
`VirtualAlloc` on Windows systems. These functions allow virtual memory to be
requested for some space, and allows us to unmap subregions so that alignment
can be guaranteed. This is done in separate regions because we found that a lot
of devices have trouble allocating enough virtual memory upfront to have the
entire heap be contiguous. By allowing it to grow virtual address space usage
with the size of the heap (and shrink as well), it works on a broader set of
devices. We chose 4 MiB as the size of segments because it was small enough to
not exhaust virtual address space, while also being large enough to not have a
lot of overhead in managing the segments.

Each heap segment is aligned to begin on a pointer address that is a multiple
of its size. For example, if segments are 4 MiB wide, then their start
addresses are aligned on a 4 MiB boundary (the low 22 bits are all 0). This
means for any pointer into the JS heap, you can get the segment start address
with a simple bitwise-and operation. All metadata for the segment is stored at
the front of the segment.

At the beginning of a segment, there is a small amount of memory reserved for
some metadata about the heap. Specifically, the following things are kept there:
- Segment ID: explained in [Compressed Pointers](#compressed-pointers)
- CardTable: explained in [Generations](#generations)
- MarkBitArray: explained in [Mark Phase](#mark-phase)
- Guard page: a page of memory that is protected from reads and writes to ensure
bugs in the VM or GC don't accidentally overwrite metadata.

The rest of the segment is free space available for JS values to be allocated
into. See
[`AlignedHeapSegment::Contents`](../include/hermes/VM/AlignedHeapSegment.h)
for details about the layout of a heap segment.

# Object Types

In order to determine reachability, we need to be able to discover
what objects point to. To do this for the variety of types in Hermes, we
define a system of metadata for each object, associated to its type which is
encoded as a `VTable`. Metadata is built once for the first Runtime created, and
is then re-used for every subsequent runtime. The metadata describes at what
offsets from the head of an object there are pointers. It also describes for
any array type where the length of the array can be found, so a dynamic number
of pointers can be found. This design allows pointers to be marked without
performing any type checks, branches, or virtual dispatch.

The `VTable *` is embedded in the header of everything allocated on the heap.
This is represented by the C++ superclass of all heap objects, called `GCCell`.
This way, given a pointer to something in the heap, its type (and pointer
metadata) can be found. The type is stored as a `CellKind` enum. The `VTable`
also includes function pointers for dynamic dispatch. This is used to implement
type-based dispatch for things like property accesses and function calls.

# Generations

GenGC is so named because it is **generational**, meaning it manages groups of
allocated objects based on their age. Age is determined by how many collections
an object has survived. GenGC has two generations: the **Young Generation** (YG)
and **Old Generation** (OG).
Allocations go initially into YG, and if they survive the first YG collection
cycle, they are promoted to OG. The OG is collected less frequently and over a
larger number of objects. YG is collected frequently because it is assumed that
there are a lot of objects that were allocated but became garbage quickly. This
is known as the
[Generational Hypothesis](https://www.memorymanagement.org/glossary/g.html#generational.hypothesis):
young objects are more likely to die than old objects.

The young generation of GenGC is a single segment, regardless of the size
of the heap as a whole. Since YG is much smaller than OG, this is also a much
shorter pause to collect YG. Since the YG is also a single segment, it's very
fast to check if a pointer is a YG pointer: get the segment start address and
compare to the YG start address. The allocation algorithm is as follows:

- Attempt to allocate into the YG segment by bumping a pointer called the
"level" of the segment up. If the level would go past the end of the segment, it
fails
- If the bump allocation succeeds, the allocation is complete and control
returns to the VM
- If the bump allocation fails, start a YG collection cycle. This will evacuate
the YG into the OG, and leave some empty space.
- Try bump allocation again after the YG collection cycle completes. If it
succeeds, return.
- Else, try to allocate directly into the OG.
- If the OG is full, start a full collection cycle (collect garbage in both the
YG and OG). Once that completes, try allocating directly into OG again.
- If all that fails, that means there's not enough space for the allocation,
raise an Out of Memory (OOM) error

The old generation of GenGC is a list of segments, each of which is created
after an allocation failed in the previous segment. The OG maintains an
allocation context which remembers the most recent active segment, and this is
where promoted YG objects are allocated into. Once the OG has reached its
configured size limit, it will do a full collection, trying to free garbage
inside the OG.

There are more details on collection cycles in the
[Collection Cycles](#collection-cycles) section.

## Allocate directly into the Old Geneneration (Pre-Tenuring)

Sometimes there are applications where the generational hypothesis doesn't hold
true, or in other words they allocate more objects that are long-lived than
short-lived. This typically occurs during the initialization period of an
application. If this rule applies to your application, you can request this
behavior from Hermes by changing the `GCConfig` that's used at Runtime
construction time:

```cpp
hermes::vm::GCConfig::Builder gcConfigBuilder{};
gcConfigBuilder
    .withAllocInYoung(false)
    .withRevertToYGAtTTI(true);
std::unique_ptr<jsi::Runtime> runtime = makeHermesRuntime(
    hermes::vm::RuntimeConfig::Builder()
        .withGCConfig(gcConfigBuilder.build())
);
```

The `AllocInYoung` parameter defaults to `true`, and controls whether
allocations go into YG by default or OG.
The `RevertToYGAtTTI` parameter defaults to `false`, and is unused unless
`AllocInYoung` is true.

See the full documentation of [GCConfig](../public/hermes/Public/GCConfig.h) for
more details and other configuration options.

# Collection Cycles

GenGC determines garbage that can be collected in a program based on what is
reachable in the graph of objects. The graph has **roots** that define the
entrypoint to the graph. These are typically things like variables on the
JS call stack. From the roots, the entire graph of objects is traversed by
following each property to another object. Think of it as similar to a
traditional graph search algorithm, although we make many optimizations specific
to the heap structure.

GenGC has two different types of garbage collection cycles
(abbreviated as "GC"). There is a Young Generation Collection or YG GC, and a
Full Collection or full GC.
A YG GC collects only objects present in the YG, and promotes objects that are
reachable to the OG so they are collected less frequently.
A full GC collects both the OG and YG simultaneously, and determines the
reachability of every object in the heap.

There are two common concepts across both types of collections:

- Mark: Marking means setting some state related to an object to say that it was
found to be reachable during the traversal of the heap. In GenGC's case, this
is often referred to as a "mark bit"
- Sweep: Sweeping means reclaiming unused space so that it can be re-used by the
allocator. This can look very different depending on the generation

## Young Generation Collection

When a YG collection is triggered, it first needs to mark the roots of the
object graph. Because YG collections occur very frequently, we want the set of
roots to be small. To achieve this we ignore any roots that are known to be
only in the OG. When each pointer is discovered for the first time, an
allocation is made in the OG for the same size, the YG object's contents are
copied, and a forwarding pointer is left in the header of the YG object pointing
to the new OG object. After that, if we see the same pointer again, we replace
it with the value of the forwarding pointer.

We also need to consider any OG objects that point into YG as part of the root
set. That is handled by a separate mechanism during JS execution, detailed below
in the section on [Write Barriers](#write-barriers).

To find all the reachable objects, we take advantage of the linear nature of
OG, and scan linearly from the original end of the allocated region, until
there aren't any more promoted objects. Each promoted object is scanned for its
own pointers. This is how all YG objects are discovered. It's similar to a
semi-space algorithm where the second space is the OG.

Once the objects are all moved, any weak references pointing to YG objects are
updated if the object is still alive, and then any finalizers are run by
iterating over a list of finalizers.

### Write Barriers

As part of the root set, we also consider any OG objects that point into YG.
To determine where these exist without iterating over the whole OG, we use a
**write barrier** whenever a pointer value is written to during JS execution.
If it would create an old -> young pointer, we want to mark the source object
as part of the root set of YG.

The way we do this is by classifying a small region of memory in the OG as a
**card**, currently 512 bytes. We determine the card for a pointer using the
same alignment trick to get the base address of the segment, then use division
to figure out the card index. The **card table** is a list of bytes stored in
the header of the segment, and when we find an object pointing into the YG, we
"dirty" its card by setting the byte value to 1. We don't use a bitmap because
the card table is only 8 KiB, and shrinking it further isn't worth it.

During YG collection time, we can quickly scan each card table for the set of
dirtied cards, then we mark all of the objects on that card. We use a separate
**card object table** to find the first object for a card. That table contains
another byte per card, except it is signed. If the sign is positive, it means
"go back that many bytes to find the first object on this card". If the sign
is negative, it means "go back that many cards and check the table at that
index". This encoding scheme allows a low memory cost way to jump across very
large objects.

Note that the way cards are dirtied means there's a chance of promoting some
YG objects that aren't actually live. This tradeoff is chosen so the cards take
up less memory and can be scanned quickly. Smaller cards are more precise, but
take up more memory and require more scanning.

## Full Collection

A full collection is much larger than a YG collection, so it happens less
frequently. It needs to determine the reachability of every object in the heap.
A full collection is triggered when the OG doesn't have enough space to satisfy
an allocation from a YG promotion or a direct-to-OG allocation.

### Mark Phase

The collection begins with a call to `markRoots`, although this time
additionally marking roots known to only point into the OG, such as the
`IdentifierTable`. The first time an object is encountered, it sets a bit in the
mark bit array, so future accesses know that it has already been found. Then we
push the newly discovered object on top of a **mark stack**.

The mark stack is drained by popping off the top object, scanning all of its
pointers, and pushing objects it discovers onto the mark stack.

There are two changes implemented to prevent the mark stack from growing without
bound:
- If a pointer to an object is discovered that is after the current address, it
isn't pushed on the mark stack. Instead, we jump to the next marked bit when
the mark stack is empty. If the pointer is before the current address, it has to
be pushed on the stack because we won't go backwards in the heap normally
- If the mark stack overflows a limit, we drop the whole stack and re-scan from
the beginning of the heap. Since many objects have already been marked, this
will go much faster than previous times. It's guaranteed to terminate because
in the worst case eventually every object will be marked and there's nothing
to collect.

Once the marking is complete this way, full reachability is known, and sweeping
can begin.

### Weak Map resolution

However, one problem to be handled are **weak references**, which are pointers
that do not keep what they point to alive. Now that we know reachability, we
know which weak pointers to make null.

But there's a special case of the JS type `WeakMap` that has to be handled:
the keys are weak, and the values are strong, unless they keep the key itself
alive. This means the values can't be marked until the key is determined
reachable through some other means.

We delay marking any WeakMap key/value pairs until the end of marking. Then for
each WeakMap, and for each key that is reachable in the weak map, we then mark
the value. If marking the value ends up making another WeakMap key reachable,
we then need to mark that. We repeat this for every key that becomes reachable
until there's nothing more to mark. At that point we can finally null out keys
that were never found to be reachable.

### Sweep & Compact Phases

Now that full reachability has been computed, we know which objects are garbage.
We want to reclaim that memory so that it can be allocated again. To do so, we
will **compact** all live memory down so that there's no space between them,
overwriting the space that was used by dead objects.

To do this, we iterate through every live object in the heap using the mark
bits, and for each one we compute its new address. We place that new address
as a forwarding pointer in the header of the object (displacing the `VTable *`
to a temporary vector to avoid losing the type information). Once a forwarding
pointer has been determined for everything in the heap, we do a second pass to
fixup pointers within objects to use the forwarded value. Finally, we copy the
memory from the previous location to the new location, overwriting whatever used
to be there.

The end result of this is that all live memory sits right next to each other in
the heap segments, and all dead memory has now been reclaimed for the allocator.
We use `madvise` on POSIX systems to tell the operating system that the leftover
memory is now not needed by the program, and it's free to be paged out and zero
filled if the OS needs to.

At this point control returns to the JS engine, typically to finish the
allocation that started the collection.
