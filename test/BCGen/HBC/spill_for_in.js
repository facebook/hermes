/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s
function f() {}
function foo() {
  // Use up available registers
  var x001 = f(), x002 = f(), x003 = f(), x004 = f(), x005 = f(), x006 = f(),
      x007 = f(), x008 = f(), x009 = f(), x010 = f(), x011 = f(), x012 = f(),
      x013 = f(), x014 = f(), x015 = f(), x016 = f(), x017 = f(), x018 = f(),
      x019 = f(), x020 = f(), x021 = f(), x022 = f(), x023 = f(), x024 = f(),
      x025 = f(), x026 = f(), x027 = f(), x028 = f(), x029 = f(), x030 = f(),
      x031 = f(), x032 = f(), x033 = f(), x034 = f(), x035 = f(), x036 = f(),
      x037 = f(), x038 = f(), x039 = f(), x040 = f(), x041 = f(), x042 = f(),
      x043 = f(), x044 = f(), x045 = f(), x046 = f(), x047 = f(), x048 = f(),
      x049 = f(), x050 = f(), x051 = f(), x052 = f(), x053 = f(), x054 = f(),
      x055 = f(), x056 = f(), x057 = f(), x058 = f(), x059 = f(), x060 = f(),
      x061 = f(), x062 = f(), x063 = f(), x064 = f(), x065 = f(), x066 = f(),
      x067 = f(), x068 = f(), x069 = f(), x070 = f(), x071 = f(), x072 = f(),
      x073 = f(), x074 = f(), x075 = f(), x076 = f(), x077 = f(), x078 = f(),
      x079 = f(), x080 = f(), x081 = f(), x082 = f(), x083 = f(), x084 = f(),
      x085 = f(), x086 = f(), x087 = f(), x088 = f(), x089 = f(), x090 = f(),
      x091 = f(), x092 = f(), x093 = f(), x094 = f(), x095 = f(), x096 = f(),
      x097 = f(), x098 = f(), x099 = f(), x100 = f(), x101 = f(), x102 = f(),
      x103 = f(), x104 = f(), x105 = f(), x106 = f(), x107 = f(), x108 = f(),
      x109 = f(), x110 = f(), x111 = f(), x112 = f(), x113 = f(), x114 = f(),
      x115 = f(), x116 = f(), x117 = f(), x118 = f(), x119 = f(), x120 = f(),
      x121 = f(), x122 = f(), x123 = f(), x124 = f(), x125 = f(), x126 = f(),
      x127 = f(), x128 = f(), x129 = f(), x130 = f(), x131 = f(), x132 = f(),
      x133 = f(), x134 = f(), x135 = f(), x136 = f(), x137 = f(), x138 = f(),
      x139 = f(), x140 = f(), x141 = f(), x142 = f(), x143 = f(), x144 = f(),
      x145 = f(), x146 = f(), x147 = f(), x148 = f(), x149 = f(), x150 = f(),
      x151 = f(), x152 = f(), x153 = f(), x154 = f(), x155 = f(), x156 = f(),
      x157 = f(), x158 = f(), x159 = f(), x160 = f(), x161 = f(), x162 = f(),
      x163 = f(), x164 = f(), x165 = f(), x166 = f(), x167 = f(), x168 = f(),
      x169 = f(), x170 = f(), x171 = f(), x172 = f(), x173 = f(), x174 = f(),
      x175 = f(), x176 = f(), x177 = f(), x178 = f(), x179 = f(), x180 = f(),
      x181 = f(), x182 = f(), x183 = f(), x184 = f(), x185 = f(), x186 = f(),
      x187 = f(), x188 = f(), x189 = f(), x190 = f(), x191 = f(), x192 = f(),
      x193 = f(), x194 = f(), x195 = f(), x196 = f(), x197 = f(), x198 = f(),
      x199 = f(), x200 = f(), x201 = f(), x202 = f(), x203 = f(), x204 = f(),
      x205 = f(), x206 = f(), x207 = f(), x208 = f(), x209 = f(), x210 = f(),
      x211 = f(), x212 = f(), x213 = f(), x214 = f(), x215 = f(), x216 = f(),
      x217 = f(), x218 = f(), x219 = f(), x220 = f(), x221 = f(), x222 = f(),
      x223 = f(), x224 = f(), x225 = f(), x226 = f(), x227 = f(), x228 = f(),
      x229 = f(), x230 = f(), x231 = f(), x232 = f(), x233 = f(), x234 = f(),
      x235 = f(), x236 = f(), x237 = f(), x238 = f(), x239 = f(), x240 = f(),
      x241 = f(), x242 = f(), x243 = f(), x244 = f(), x245 = f(), x246 = f(),
      x247 = f(), x248 = f(), x249 = f(), x250 = f(), x251 = f(), x252 = f(),
      x253 = f(), x254 = f(), x255 = f(), x256 = f();


// Now try to GetNextPName and ensure that values are spilled after.
// This instruction is especially interesting because it's a terminator
// with multiple branches (in IR), so we need to insert spilling code
// in all of them while also not ruining Phi nodes.

//CHECK:      {{.*}} GetNextPName {{.*}}
//CHECK-NEXT: {{.*}} JmpUndefined {{.*}}
//CHECK-NEXT: {{.*}} MovLong {{.*}}
//CHECK-NEXT: {{.*}} MovLong {{.*}}
//CHECK-NEXT: {{.*}} MovLong {{.*}}
//CHECK-NEXT: {{.*}} MovLong {{.*}}
  var count=0;
  for(var arg in count) {
    count++;
  }

  // Make sure registers are still in use
  return x001 + x002 + x003 + x004 + x005 + x006 + x007 + x008 + x009 + x010 +
    x011 + x012 + x013 + x014 + x015 + x016 + x017 + x018 + x019 + x020 + x021
    + x022 + x023 + x024 + x025 + x026 + x027 + x028 + x029 + x030 + x031 +
    x032 + x033 + x034 + x035 + x036 + x037 + x038 + x039 + x040 + x041 + x042
    + x043 + x044 + x045 + x046 + x047 + x048 + x049 + x050 + x051 + x052 +
    x053 + x054 + x055 + x056 + x057 + x058 + x059 + x060 + x061 + x062 + x063
    + x064 + x065 + x066 + x067 + x068 + x069 + x070 + x071 + x072 + x073 +
    x074 + x075 + x076 + x077 + x078 + x079 + x080 + x081 + x082 + x083 + x084
    + x085 + x086 + x087 + x088 + x089 + x090 + x091 + x092 + x093 + x094 +
    x095 + x096 + x097 + x098 + x099 + x100 + x101 + x102 + x103 + x104 + x105
    + x106 + x107 + x108 + x109 + x110 + x111 + x112 + x113 + x114 + x115 +
    x116 + x117 + x118 + x119 + x120 + x121 + x122 + x123 + x124 + x125 + x126
    + x127 + x128 + x129 + x130 + x131 + x132 + x133 + x134 + x135 + x136 +
    x137 + x138 + x139 + x140 + x141 + x142 + x143 + x144 + x145 + x146 + x147
    + x148 + x149 + x150 + x151 + x152 + x153 + x154 + x155 + x156 + x157 +
    x158 + x159 + x160 + x161 + x162 + x163 + x164 + x165 + x166 + x167 + x168
    + x169 + x170 + x171 + x172 + x173 + x174 + x175 + x176 + x177 + x178 +
    x179 + x180 + x181 + x182 + x183 + x184 + x185 + x186 + x187 + x188 + x189
    + x190 + x191 + x192 + x193 + x194 + x195 + x196 + x197 + x198 + x199 +
    x200 + x201 + x202 + x203 + x204 + x205 + x206 + x207 + x208 + x209 + x210
    + x211 + x212 + x213 + x214 + x215 + x216 + x217 + x218 + x219 + x220 +
    x221 + x222 + x223 + x224 + x225 + x226 + x227 + x228 + x229 + x230 + x231
    + x232 + x233 + x234 + x235 + x236 + x237 + x238 + x239 + x240 + x241 +
    x242 + x243 + x244 + x245 + x246 + x247 + x248 + x249 + x250 + x251 + x252
    + x253 + x254 + x255 + x256;
}
