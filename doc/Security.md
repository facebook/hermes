# Security in the Hermes VM

The Hermes compiler and VM are designed to allow third party developers to
execute JavaScript source code and bytecode. This features opens the door for
attacks by developers that can submit malicious code that would exploit the VM.
The VM is always embedded in the context of a larger application and it is
critical for the VM to be able to protect the containing app from malicious code
running in the VM. We also assume that we can't separate the VM into a dedicated
process or address space and need to live in the same address space as the
container. 

In this document we assume that the threat that we are protecting ourselves
against can be reduced to the requirement of not allowing the running JavaScript
code to read or write memory beyond the memory that's allocated by the VM and
dedicated to the execution of the JavaScript program according to the
specification of the language. We do not attempt to verify the correctness of
the program, what may include infinite loops or various kinds of attacks on the
host application by means of using the standard API. 

This document describes the layers of protection that we use, or plan to
implement in order to mitigate the security risk associated with running foreign
JavaScript code in the same address space as an application that contains
sensitive information. 

Software quality In the Hermes compiler uses modern software techniques to
ensure the correctness of the software and to eliminate basic software bugs,
such as buffer overflow, that can lead to security holes. Hermes uses the LLVM
compiler toolkit that provides data structures that are extensively tested and
reviewed by a large community of developers. The Hermes compiler is executed
with Clang's address sanitizer, undefined-behavior sanitizer and memory
sanitizer to ensure the correctness of the compiler and VM, to make sure that
there are no out-of-bound calls, memory leaks and other kinds of security bugs.
The compiler is fuzzed using lib-fuzzed to detect errors and edge cases.

Bytecode verification We intend to implement bytecode verification to ensure
that the bytecode that's loaded by the VM is correct and ready to be executed
without memory safety problems. The verifier will include checks that: Register
operands are legal and that the memory that they reference is properly
initialized and guarded.  All basic blocks are terminated and that the control
flow is predictable.  Jump instructions don't jump into the middle of an
existing instruction (and circumvent the verifier).

