// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -dump-bytecode -target=HBC -non-strict %s | %FileCheck --match-full-lines %s

print(/a\x01\u017f/i);
print(/a\x01\u017f/i);

// CHECK: RegExp Bytecodes:
// CHECK:       0: /a\x01\u017f/i
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 1 constraints: 5
// CHECK-NEXT:    0000  MatchCharICase8: 'A'
// CHECK-NEXT:    0002  MatchCharICase8: 0x01
// CHECK-NEXT:    0004  MatchCharICase16: 0x17f
// CHECK-NEXT:    0007  Goal

print(/^a\u017f\x01$/);
// CHECK:       1: /^a\u017f\x01$/
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 0 constraints: 7
// CHECK-NEXT:    0000  LeftAnchor
// CHECK-NEXT:    0001  MatchChar8: 'a'
// CHECK-NEXT:    0003  MatchChar16: 0x17f
// CHECK-NEXT:    0006  MatchChar8: 0x01
// CHECK-NEXT:    0008  RightAnchor
// CHECK-NEXT:    0009  Goal

print(/^a|b/);
// CHECK:       2: /^a|b/
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 0 constraints: 4
// CHECK-NEXT:    0000  Alternation: Target 0x0f, constraints 6,4
// CHECK-NEXT:    0007  LeftAnchor
// CHECK-NEXT:    0008  MatchChar8: 'a'
// CHECK-NEXT:    000a  Jump32: 0x11
// CHECK-NEXT:    000f  MatchChar8: 'b'
// CHECK-NEXT:    0011  Goal

print(/[a-z][^A-Z0-9_\d][\s][abc]/);
// CHECK:       3: /[a-z][^A-Z0-9_\d][\s][abc]/
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 0 constraints: 4
// CHECK-NEXT:    0000  Bracket: [a-z]
// CHECK-NEXT:    000a  Bracket: [^\dA-Z0-9_]
// CHECK-NEXT:    001c  Bracket: [\s]
// CHECK-NEXT:    0022  Bracket: [abc]
// CHECK-NEXT:    0034  Goal

print(/a(b(c)(d))e\1\2/);
// CHECK:       4: /a(b(c)(d))e\1\2/
// CHECK-NEXT:    Header: marked: 3 loops: 0 flags: 0 constraints: 4
// CHECK-NEXT:    0000  MatchChar8: 'a'
// CHECK-NEXT:    0002  BeginMarkedSubexpression: 1
// CHECK-NEXT:    0005  MatchChar8: 'b'
// CHECK-NEXT:    0007  BeginMarkedSubexpression: 2
// CHECK-NEXT:    000a  MatchChar8: 'c'
// CHECK-NEXT:    000c  EndMarkedSubexpression: 2
// CHECK-NEXT:    000f  BeginMarkedSubexpression: 3
// CHECK-NEXT:    0012  MatchChar8: 'd'
// CHECK-NEXT:    0014  EndMarkedSubexpression: 3
// CHECK-NEXT:    0017  EndMarkedSubexpression: 1
// CHECK-NEXT:    001a  MatchChar8: 'e'
// CHECK-NEXT:    001c  BackRefInsn: 1
// CHECK-NEXT:    001f  BackRefInsn: 2
// CHECK-NEXT:    0022  Goal

print(/\b\B/);
// CHECK:       5: /\b\B/
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 0 constraints: 0
// CHECK-NEXT:    0000  WordBoundary: \b
// CHECK-NEXT:    0002  WordBoundary: \B
// CHECK-NEXT:    0004  Goal

print(/abc(?=^)(?!def)/i);
// CHECK:       6: /abc(?=^)(?!def)/i
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 1 constraints: 6
// CHECK-NEXT:    0000  MatchCharICase8: 'A'
// CHECK-NEXT:    0002  MatchCharICase8: 'B'
// CHECK-NEXT:    0004  MatchCharICase8: 'C'
// CHECK-NEXT:    0006  Lookahead: = (constraints: 2, marked expressions=[0,0), continuation 0x13)
// CHECK-NEXT:    0011  LeftAnchor
// CHECK-NEXT:    0012  Goal
// CHECK-NEXT:    0013  Lookahead: ! (constraints: 4, marked expressions=[0,0), continuation 0x25)
// CHECK-NEXT:    001e  MatchCharICase8: 'D'
// CHECK-NEXT:    0020  MatchCharICase8: 'E'
// CHECK-NEXT:    0022  MatchCharICase8: 'F'
// CHECK-NEXT:    0024  Goal
// CHECK-NEXT:    0025  Goal

print(/ab*c+d{3,5}/);
// CHECK:        7: /ab*c+d{3,5}/
// CHECK-NEXT:    Header: marked: 0 loops: 3 flags: 0 constraints: 4
// CHECK-NEXT:    0000  MatchChar8: 'a'
// CHECK-NEXT:    0002  Width1Loop: 0 greedy {0, 4294967295}
// CHECK-NEXT:    0014  MatchChar8: 'b'
// CHECK-NEXT:    0016  Width1Loop: 1 greedy {1, 4294967295}
// CHECK-NEXT:    0028  MatchChar8: 'c'
// CHECK-NEXT:    002a  Width1Loop: 2 greedy {3, 5}
// CHECK-NEXT:    003c  MatchChar8: 'd'
// CHECK-NEXT:    003e  Goal

print(/a((b+){3})*/);
// CHECK:        8: /a((b+){3})*/
// CHECK-NEXT:    Header: marked: 2 loops: 3 flags: 0 constraints: 4
// CHECK-NEXT:     0000  MatchChar8: 'a'
// CHECK-NEXT:     0002  BeginLoop: 2 greedy {0, 4294967295} (constraints: 4)
// CHECK-NEXT:     001d  BeginMarkedSubexpression: 1
// CHECK-NEXT:     0020  BeginLoop: 1 greedy {3, 3} (constraints: 4)
// CHECK-NEXT:     003b  BeginMarkedSubexpression: 2
// CHECK-NEXT:     003e  Width1Loop: 0 greedy {1, 4294967295}
// CHECK-NEXT:     0050  MatchChar8: 'b'
// CHECK-NEXT:     0052  EndMarkedSubexpression: 2
// CHECK-NEXT:     0055  EndLoop: 0x20
// CHECK-NEXT:     005a  EndMarkedSubexpression: 1
// CHECK-NEXT:     005d  EndLoop: 0x02
// CHECK-NEXT:     0062  Goal

print(/(^b)+(c)*?/);
// CHECK:        9: /(^b)+(c)*?/
// CHECK-NEXT:    Header: marked: 2 loops: 2 flags: 0 constraints: 6
// CHECK-NEXT:     0000  BeginLoop: 0 greedy {1, 4294967295} (constraints: 6)
// CHECK-NEXT:     001b  BeginMarkedSubexpression: 1
// CHECK-NEXT:     001e  LeftAnchor
// CHECK-NEXT:     001f  MatchChar8: 'b'
// CHECK-NEXT:     0021  EndMarkedSubexpression: 1
// CHECK-NEXT:     0024  EndLoop: 0x00
// CHECK-NEXT:     0029  BeginLoop: 1 nongreedy {0, 4294967295} (constraints: 4)
// CHECK-NEXT:     0044  BeginMarkedSubexpression: 2
// CHECK-NEXT:     0047  MatchChar8: 'c'
// CHECK-NEXT:     0049  EndMarkedSubexpression: 2
// CHECK-NEXT:     004c  EndLoop: 0x29
// CHECK-NEXT:     0051  Goal

print(/[\u017f]/i);
// CHECK:        10: /[\u017f]/i
// CHECK-NEXT:    Header: marked: 0 loops: 0 flags: 1 constraints: 5
// CHECK-NEXT:     0000  Bracket: [0x17f]
// CHECK-NEXT:     000a  Goal

print(/a*/);
// CHECK:        11: /a*/
// CHECK-NEXT:    Header: marked: 0 loops: 1 flags: 0 constraints: 0
// CHECK-NEXT:     0000  Width1Loop: 0 greedy {0, 4294967295}
// CHECK-NEXT:     0012  MatchChar8: 'a'
// CHECK-NEXT:     0014  Goal

print(/a+/);
// CHECK:        12: /a+/
// CHECK-NEXT:    Header: marked: 0 loops: 1 flags: 0 constraints: 4
// CHECK-NEXT:    0000  Width1Loop: 0 greedy {1, 4294967295}
// CHECK-NEXT:    0012  MatchChar8: 'a'
// CHECK-NEXT:    0014  Goal

print(/(?:a+)*/);
// CHECK:        13: /(?:a+)*/
// CHECK-NEXT:    Header: marked: 0 loops: 2 flags: 0 constraints: 0
// CHECK-NEXT:     0000  BeginSimpleLoop: (constraints: 4)
// CHECK-NEXT:     0006  Width1Loop: 0 greedy {1, 4294967295}
// CHECK-NEXT:     0018  MatchChar8: 'a'
// CHECK-NEXT:     001a  EndSimpleLoop: 0x00
// CHECK-NEXT:     001f  Goal

print(/(?:a*)*/);
// CHECK:        14: /(?:a*)*/
// CHECK-NEXT:    Header: marked: 0 loops: 2 flags: 0 constraints: 0
// CHECK-NEXT:     0000  BeginLoop: 1 greedy {0, 4294967295} (constraints: 0)
// CHECK-NEXT:     001b  Width1Loop: 0 greedy {0, 4294967295}
// CHECK-NEXT:     002d  MatchChar8: 'a'
// CHECK-NEXT:     002f  EndLoop: 0x00
// CHECK-NEXT:     0034  Goal

print(/[a-zA-Z]*/);
// CHECK:        15: /[a-zA-Z]*/
// CHECK-NEXT:    Header: marked: 0 loops: 1 flags: 0 constraints: 0
// CHECK-NEXT:     0000  Width1Loop: 0 greedy {0, 4294967295}
// CHECK-NEXT:     0012  Bracket: [a-zA-Z]
// CHECK-NEXT:     0020  Goal

print(/.*/);
// CHECK:        16: /.*/
// CHECK-NEXT:   Header: marked: 0 loops: 1 flags: 0 constraints: 0
// CHECK-NEXT:    0000  Width1Loop: 0 greedy {0, 4294967295}
// CHECK-NEXT:    0012  MatchAnyButNewline
// CHECK-NEXT:    0013  Goal

print(/a*/i);
// CHECK:        17: /a*/i
// CHECK-NEXT:   Header: marked: 0 loops: 1 flags: 1 constraints: 0
// CHECK-NEXT:    0000  Width1Loop: 0 greedy {0, 4294967295}
// CHECK-NEXT:    0012  MatchCharICase8: 'A'
// CHECK-NEXT:    0014  Goal

print(/(a)(?=(.))/i);
// CHECK:        18: /(a)(?=(.))/i
// CHECK-NEXT:   Header: marked: 2 loops: 0 flags: 1 constraints: 4
// CHECK-NEXT:   0000  BeginMarkedSubexpression: 1
// CHECK-NEXT:   0003  MatchCharICase8: 'A'
// CHECK-NEXT:   0005  EndMarkedSubexpression: 1
// CHECK-NEXT:   0008  Lookahead: = (constraints: 4, marked expressions=[1,2), continuation 0x1b)
// CHECK-NEXT:   0013  BeginMarkedSubexpression: 2
// CHECK-NEXT:   0016  MatchAnyButNewline
// CHECK-NEXT:   0017  EndMarkedSubexpression: 2
// CHECK-NEXT:   001a  Goal
// CHECK-NEXT:   001b  Goal

// There are 255 'a's here.
print(/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaoverflow/);
// CHECK:        19: /{{a{255}overflow}}/
// CHECK-NEXT:   Header: marked: 0 loops: 0 flags: 0 constraints: 4
// CHECK-NEXT:   0000  MatchNChar8: {{'a{255}'}}
// CHECK-NEXT:   0101  MatchNChar8: 'overflow'
// CHECK-NEXT:   010b  Goal
