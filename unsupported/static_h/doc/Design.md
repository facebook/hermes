---
id: design
title: Design Overview
---

This is the design document for the Hermes Engine.

## JSON estree parser

## JavaScript parser

## IRGen

## Hermes Bytecode Generator

The Hermes bytecode generator is responsible for translating the
Hermes high-level IR into Hermes bytecode. This section only describes the
generation of the opcode stream, not the whole bundle file format that is
described in a separate document.

The bytecode is a register-based bytecode. The number of registers is
infinite, but there are some restrictions on the registers. For example,
registers that are allocated to call instructions must be consecutive, and most
instructions only accept an 8-bit register index.

The first phase of bytecode generation is lowering of some instructions to
target-specific instructions. Next, the register allocator allocates
registers for each instruction in the lowered IR. Notice that the Hermes IR
has a single return value per instruction. We model multiple return values or
in-out variables using alloc-stack instructions.

One interesting design decision here is that register allocation happens on the
IR prior to instruction selection. Performing register allocation prior to
lowering is often done in JIT compilers where the lowering phase is trivial and
close to a 1:1 translation between high-level IR and the low-level target IR.
The lowered Hermes IR is very close to the bytecode format with a few
exceptions: the handling of variables, the representation of constants and the
representation of stack-allocated variables. Stack allocated registers are
implemented as values that are pinned into a specific registers (at least during
the lifetime of the stack allocation). Load/Store instructions from the stack
allocation values are lowered into MOV instructions.

The current register allocation implementation is a very simple linear scan. The
allocation has four main steps. First, we number the instructions in the
functions, and traverse the basic blocks in the function in reverse-post-order
scan.  Second, we calculate the liveness graph of the result of each
instruction in the function. We propagate this information across the function
with a simple data-flow scan.  Third, we calculate the liveness interval of each
instruction. Finally, we scan the instructions in the function one by one and
assign registers. We maintain a list of currently live intervals that correspond
to the register file. We represent the register file with a simple bit vector
and prefer to allocate registers from the beginning of the file.  When we run out
of registers we simply allocate new registers. When we reach instructions that
require target-specific handling we call the backend to fill in the details. For
example, the Hermes backend handles call instructions in a special way.  At the
moment we do not coalesce values and our PHI nodes generate two MOVs.

After register allocation we lower the Hermes IR into pseudo opcodes using a
one-to-one or one-to-many translation. There are a few interesting optimizations
during this translation.  The first interesting optimization is the optimization
that eliminates redundant constant jumps. It is possible to eliminate many
branches by scheduling basic blocks in a way that maximizes the number of
fall-through opportunities. The current algorithm uses a greedy approach where
we emit basic blocks in some order (that starts with the entry block), and
attempt to place the fall-through destination right after terminators that
branch (conditional and unconditional branches) if the block has not been
generated already. In the future we may need to investigate if this greedy
algorithm is optimal.

When we generate the opcode stream we need to encode jump targets. However, when
we emit opcodes that refer to jump destinations that we have not yet emitted
then we do not know the address of the destination. We solve this problem by
emitting a dummy value and keeping a side-table that saves the location that we
need to patch. When we finish emitting all of the opcodes we scan the side table
and patch all of the locations that refer to previously unresolved addresses.

### Hermes Bytecode Instructions

Hermes bytecode adopts variable-length instructions. Each operand to a bytecode
instruction has a fixed-type and width, defined by the opcode. For instance,
Jmp takes a 1-byte offset as the jump target, while JmpLong takes a 4-byte
offset as the jump target.  Fixed-type/width instructions allow us to decode
them efficiently in the interpreter.
However we are trading off with an increasing number of
opcodes to handle different operand widths (e.g. two Jmp opcodes instead of
one). We believe that we are able to avoid opcode explosion by generating the
code smartly. A full list of Hermes bytecode opcodes can be found in
BytecodeList.def. There are a few interesting design decisions worth mentioning
here:
- Registers: We discovered that in all of the Facebook mobile JS code as well as
majority of external benchmarks, no function ever uses more than 256 registers.
Hence we always use 1-byte to represent register index, which will be most
efficient for the normal cases. Spilling is implemented via MovLong which
supports 32 bit register indices.
- Constants: we achieve constant loading fully through instructions.
For fixed-value constants such as undefined, null, true and false, we introduce
a corresponding load opcode for each of them into a register (e.g.
LoadConstUndefined `dstReg`); For 32-bit integers, we introduce LoadConstInt
opcode, which takes a 4-bytes immediate value and load it into a register; for
doubles, we introduce LoadConstDouble, which takes a 8-bytes immediate value
and load it into a register; finally for strings, we introduce LoadConstString,
which takes an index to the string table from which to load into a register.
Doing so can significantly reduce the size of the bytecode, however it does
introduce a few more opcodes which could slow down the interpreter.
- Non-local Variable Access: Local variables are translated to registers.
Non-local variables are variables from different scopes/environments. Without a
compiler, accessing non-local variables in JavaScript usually means a scope
lookup (i.e. locating the closest scope in the scope chain that defines the
variable), followed by a symbol lookup in that scope.
However with a compiler, it is possible to statically determine the scope of
every variable, and hence there is no need for a real scope lookup. In Hermes
backend, for each non-local variable access, we simply calculate the delta
between the defining scope and the current scope, and locate such scope using
the delta (i.e. number of times the VM needs to follow the scope chain)
directly. Furthermore, since we know exactly what and how many variables are
defined in each scope (except the global scope), we skip the symbol lookup in
the scope but instead using a direct index access to retrieve such variable in
the located scope. We believe that skipping both scope lookups and symbol
lookups can significantly improve the runtime performance.

### Bytecode File Format

The bytecode file contains the bytecode as well as necessary metadata and
auxiliary data sections for the VM to execute properly. The file format is
defined in BytecodeFileFormat.h, and structures as following:
- FILE HEADER: The file header contains the MAGIC, the current format version,
and a list of global metadata, including the file size, offset of the function
header table, offset of the string table, index of the global code and number
of functions.
- FUNCTION HEADER TABLE: This is a list of function headers. Each function header
contains metadata of a function, such as the offset of the function bytecode in
the file, number of parameters, size of the frame/environment, size of the
bytecode and etc. This list also naturally assigns an index to each function in
the file, which makes access to each function convenient in the VM.
- STRING TABLE&STORAGE: All the strings used in all functions are uniqued and
stored in the section to avoid redundant string storage. This section contains
two parts: The string storage, which is a long sequence of raw characters; the
string table, which is a list of pairs, each pair represents a string through
the offset to the string storage and the length of the string. The string table
also naturally assigns an index to each unique string, which makes it
convenient to refer to the strings in the bytecode.
- FUNCTION BYTECODES: This section is the core section of the bytecode file,
containing a list of compiled function body. The function body contains its
executable bytecode, along with a few tables that are used by the bytecode,
including the exception handler table (tells where to jump to when exception
happens), the array buffer (used to initialize constant arrays). Likely there
will be a few more tables coming to support RegExp and debug information.

### Serialization / Deserialization

Because the Hermes backend is in the same codebase as the VM, there are
opportunities for us to share code between them. In particular, we want the
serialization in the backend to be able to share the same target data structure
as the deserialization in the VM. This introduces some interesting questions:
- How can we share data/code without having to link too much code on each side?
- How to avoid data copies during both serialization and deserialization?

We introduce two design pieces to achieve code sharing efficiently:
- Generator: During serialization, we often need many auxiliary data and
functionality to aid the process, though many of that will not be needed in the
end. To ensure the separation of the complexity and allow the deserialization
to share data/code most efficiently, we use a BytecodeModuleGenerator and a
list of BytecodeFunctionGenerator for the purpose of generating bytecode and
serialization. After all the processing, they will eventually generate a
BytecodeModule and a list of BytecodeFunction that contains minimum amount of
data/functionality required to generate the bytecode file. Hence we can share
this minimum data structure between the backend and the VM.
- StreamVector: During serialization, we need to move/copy the part of the
auxiliary data  from the generator to the shared minimum data structure; during
deserialization, we need to move/copy the content of the file into the shared
minimum data structure. Both can be expensive if not managed properly. To
minimize the copying overhead, we abstracted the complexity using a class named
StreamVector. During serialization, StreamVector allows us to move the data out
of the generator without copying (through std::vector::swap); during
deserialization, StreamVector allows us to take in a raw data pointer to the
memory buffer from the file directly, without having to copy them too.

### Interaction with the VM

At runtime, the VM will deserialize the bytecode from the file and interpret
it. A few components are involved:
BytecodeModule: This is the in-memory representation of the whole bytecode
file, containing all the bytecode functions. During both serialization and
deserialization, this data structure is generated, as a static representation
of the whole file.
- BytecodeFunction: This is the in-memory representation of a function’s
bytecode.
- RuntimeModule: This is the dynamic version of the bytecode module, containing
necessary runtime information for interpretation.
- CodeBlock: This is the dynamic version of the bytecode function, containing
necessary runtime information to execute a function.
- Domain: A GC-managed proxy which references a set of RuntimeModules, acting as
a bridge between the GC heap and the C++ heap.
- JSFunction: This is the Function object in JavaScript.

It is important (and tricky) to efficiently manage the memory and ownership of
these objects properly at runtime. The following figure demonstrates it. Own
means one manages the memory of the other through unique_ptr; Pointer means one
has a raw pointer to the other, without any ownership. Indirectly Own is a
special kind of ownership, which will be explained below. To summarize the
ownership:
- JSFunction is a JavaScript object, and hence managed by the heap/garbage
collector directly.
- JSFunction owns a GC-visible reference to a Domain, and a pointer to the
corresponding CodeBlock to execute.
- CodeBlock contains a pointer to both the RuntimeModule to access runtime
information, as well as a pointer to the corresponding BytecodeFunction which
contains the static function bytecode to execute.
- BytecodeModule owns a list of BytecodeFunction.
- RuntimeModule owns a list of CodeBlock, as well as the corresponding
BytecodeModule.
- Domain owns one or more RuntimeModule. This is the mechanism by which an extant
JSFunction keeps the backing bytecode alive.

## Interpreter

## Garbage Collector
