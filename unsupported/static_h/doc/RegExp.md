---
id: regexp
title: RegExp
---

## RegExp

The Hermes regexp engine is a traditional engine using a backtracking stack. It compiles a regexp into bytecode which can be executed efficiently. For regexp literals like `/abc/`, this occurs at compile time: the regexp bytecode is embedded into the Hermes bytecode file. Note regexp bytecode is distinct from Hermes bytescode.

The regexp engine proceeds as follows:

1. *Parse phase.* The regexp parser emits a tree of nodes, effectively an IR.
1. *Optimization phase.* The node tree is traversed and optimized in various ways.
1. *Emitting phase.* The node tree is traversed and emits regexp bytecode.
1. *Execution phase.* The bytecode is executed against an input string.

## Supported Syntax

As of this writing, Hermes regexp supports

1. All of ES6, including global, case-insensitive, multiline, sticky, and Unicode (and legacy).
1. ES9 lookbehinds.

Missing features from ES9 include:

1. Named capture groups.
1. Unicode property escapes.
