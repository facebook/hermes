/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -reuse-prop-cache -target=HBC -dump-bytecode %s -O | %FileCheckOrRegen %s --match-full-lines

function manyGetByIds(o, o2) {
  var sum = 0;
  sum +=
    // First 100
    o.p0 + o.p1 + o.p2 + o.p3 + o.p4 + o.p5 + o.p6 + o.p7 + o.p8 + o.p9 +
    o.p10 + o.p11 + o.p12 + o.p13 + o.p14 + o.p15 + o.p16 + o.p17 + o.p18 + o.p19 +
    o.p20 + o.p21 + o.p22 + o.p23 + o.p24 + o.p25 + o.p26 + o.p27 + o.p28 + o.p29 +
    o.p30 + o.p31 + o.p32 + o.p33 + o.p34 + o.p35 + o.p36 + o.p37 + o.p38 + o.p39 +
    o.p40 + o.p41 + o.p42 + o.p43 + o.p44 + o.p45 + o.p46 + o.p47 + o.p48 + o.p49 +
    o.p50 + o.p51 + o.p52 + o.p53 + o.p54 + o.p55 + o.p56 + o.p57 + o.p58 + o.p59 +
    o.p60 + o.p61 + o.p62 + o.p63 + o.p64 + o.p65 + o.p66 + o.p67 + o.p68 + o.p69 +
    o.p70 + o.p71 + o.p72 + o.p73 + o.p74 + o.p75 + o.p76 + o.p77 + o.p78 + o.p79 +
    o.p80 + o.p81 + o.p82 + o.p83 + o.p84 + o.p85 + o.p86 + o.p87 + o.p88 + o.p89 +
    o.p90 + o.p91 + o.p92 + o.p93 + o.p94 + o.p95 + o.p96 + o.p97 + o.p98 + o.p99 +
    // Second 100
    o.p100 + o.p101 + o.p102 + o.p103 + o.p104 +
    o.p105 + o.p106 + o.p107 + o.p108 + o.p109 +
    o.p110 + o.p111 + o.p112 + o.p113 + o.p114 +
    o.p115 + o.p116 + o.p117 + o.p118 + o.p119 +
    o.p120 + o.p121 + o.p122 + o.p123 + o.p124 +
    o.p125 + o.p126 + o.p127 + o.p128 + o.p129 +
    o.p130 + o.p131 + o.p132 + o.p133 + o.p134 +
    o.p135 + o.p136 + o.p137 + o.p138 + o.p139 +
    o.p140 + o.p141 + o.p142 + o.p143 + o.p144 +
    o.p145 + o.p146 + o.p147 + o.p148 + o.p149 +
    o.p150 + o.p151 + o.p152 + o.p153 + o.p154 +
    o.p155 + o.p156 + o.p157 + o.p158 + o.p159 +
    o.p160 + o.p161 + o.p162 + o.p163 + o.p164 +
    o.p165 + o.p166 + o.p167 + o.p168 + o.p169 +
    o.p170 + o.p171 + o.p172 + o.p173 + o.p174 +
    o.p175 + o.p176 + o.p177 + o.p178 + o.p179 +
    o.p180 + o.p181 + o.p182 + o.p183 + o.p184 +
    o.p185 + o.p186 + o.p187 + o.p188 + o.p189 +
    o.p190 + o.p191 + o.p192 + o.p193 + o.p194 +
    o.p195 + o.p196 + o.p197 + o.p198 + o.p199 +
    // Last 55
    o.p200 + o.p201 + o.p202 + o.p203 + o.p204 +
    o.p205 + o.p206 + o.p207 + o.p208 + o.p209 +
    o.p210 + o.p211 + o.p212 + o.p213 + o.p214 +
    o.p215 + o.p216 + o.p217 + o.p218 + o.p219 +
    o.p220 + o.p221 + o.p222 + o.p223 + o.p224 +
    o.p225 + o.p226 + o.p227 + o.p228 + o.p229 +
    o.p230 + o.p231 + o.p232 + o.p233 + o.p234 +
    o.p235 + o.p236 + o.p237 + o.p238 + o.p239 +
    o.p240 + o.p241 + o.p242 + o.p243 + o.p244 +
    o.p245 + o.p246 + o.p247 + o.p248 + o.p249 +
    o.p250 + o.p251 + o.p252 + o.p253 + o.p254;
  // Repeat p254 on o2; they should share the same cache index (254).
  sum += o2.p254;
  // now more properties should use the overflow default index (255)
  sum += o.p255;
  sum += o.p256;
  sum += o.p257;
  return sum;
}

function manyPutByIds(o, o2, val) {
  // First 100.
  o.p0 = val; o.p1 = val; o.p2 = val; o.p3 = val; o.p4 = val;
  o.p5 = val; o.p6 = val; o.p7 = val; o.p8 = val; o.p9 = val;
  o.p10 = val; o.p11 = val; o.p12 = val; o.p13 = val; o.p14 = val;
  o.p15 = val; o.p16 = val; o.p17 = val; o.p18 = val; o.p19 = val;
  o.p20 = val; o.p21 = val; o.p22 = val; o.p23 = val; o.p24 = val;
  o.p25 = val; o.p26 = val; o.p27 = val; o.p28 = val; o.p29 = val;
  o.p30 = val; o.p31 = val; o.p32 = val; o.p33 = val; o.p34 = val;
  o.p35 = val; o.p36 = val; o.p37 = val; o.p38 = val; o.p39 = val;
  o.p40 = val; o.p41 = val; o.p42 = val; o.p43 = val; o.p44 = val;
  o.p45 = val; o.p46 = val; o.p47 = val; o.p48 = val; o.p49 = val;
  o.p50 = val; o.p51 = val; o.p52 = val; o.p53 = val; o.p54 = val;
  o.p55 = val; o.p56 = val; o.p57 = val; o.p58 = val; o.p59 = val;
  o.p60 = val; o.p61 = val; o.p62 = val; o.p63 = val; o.p64 = val;
  o.p65 = val; o.p66 = val; o.p67 = val; o.p68 = val; o.p69 = val;
  o.p70 = val; o.p71 = val; o.p72 = val; o.p73 = val; o.p74 = val;
  o.p75 = val; o.p76 = val; o.p77 = val; o.p78 = val; o.p79 = val;
  o.p80 = val; o.p81 = val; o.p82 = val; o.p83 = val; o.p84 = val;
  o.p85 = val; o.p86 = val; o.p87 = val; o.p88 = val; o.p89 = val;
  o.p90 = val; o.p91 = val; o.p92 = val; o.p93 = val; o.p94 = val;
  o.p95 = val; o.p96 = val; o.p97 = val; o.p98 = val; o.p99 = val;
  // Second 100
  o.p100 = val; o.p101 = val; o.p102 = val; o.p103 = val; o.p104 = val;
  o.p105 = val; o.p106 = val; o.p107 = val; o.p108 = val; o.p109 = val;
  o.p110 = val; o.p111 = val; o.p112 = val; o.p113 = val; o.p114 = val;
  o.p115 = val; o.p116 = val; o.p117 = val; o.p118 = val; o.p119 = val;
  o.p120 = val; o.p121 = val; o.p122 = val; o.p123 = val; o.p124 = val;
  o.p125 = val; o.p126 = val; o.p127 = val; o.p128 = val; o.p129 = val;
  o.p130 = val; o.p131 = val; o.p132 = val; o.p133 = val; o.p134 = val;
  o.p135 = val; o.p136 = val; o.p137 = val; o.p138 = val; o.p139 = val;
  o.p140 = val; o.p141 = val; o.p142 = val; o.p143 = val; o.p144 = val;
  o.p145 = val; o.p146 = val; o.p147 = val; o.p148 = val; o.p149 = val;
  o.p150 = val; o.p151 = val; o.p152 = val; o.p153 = val; o.p154 = val;
  o.p155 = val; o.p156 = val; o.p157 = val; o.p158 = val; o.p159 = val;
  o.p160 = val; o.p161 = val; o.p162 = val; o.p163 = val; o.p164 = val;
  o.p165 = val; o.p166 = val; o.p167 = val; o.p168 = val; o.p169 = val;
  o.p170 = val; o.p171 = val; o.p172 = val; o.p173 = val; o.p174 = val;
  o.p175 = val; o.p176 = val; o.p177 = val; o.p178 = val; o.p179 = val;
  o.p180 = val; o.p181 = val; o.p182 = val; o.p183 = val; o.p184 = val;
  o.p185 = val; o.p186 = val; o.p187 = val; o.p188 = val; o.p189 = val;
  o.p190 = val; o.p191 = val; o.p192 = val; o.p193 = val; o.p194 = val;
  o.p195 = val; o.p196 = val; o.p197 = val; o.p198 = val; o.p199 = val;
  // Last 55
  o.p200 = val; o.p201 = val; o.p202 = val; o.p203 = val; o.p204 = val;
  o.p205 = val; o.p206 = val; o.p207 = val; o.p208 = val; o.p209 = val;
  o.p210 = val; o.p211 = val; o.p212 = val; o.p213 = val; o.p214 = val;
  o.p215 = val; o.p216 = val; o.p217 = val; o.p218 = val; o.p219 = val;
  o.p220 = val; o.p221 = val; o.p222 = val; o.p223 = val; o.p224 = val;
  o.p225 = val; o.p226 = val; o.p227 = val; o.p228 = val; o.p229 = val;
  o.p230 = val; o.p231 = val; o.p232 = val; o.p233 = val; o.p234 = val;
  o.p235 = val; o.p236 = val; o.p237 = val; o.p238 = val; o.p239 = val;
  o.p240 = val; o.p241 = val; o.p242 = val; o.p243 = val; o.p244 = val;
  o.p245 = val; o.p246 = val; o.p247 = val; o.p248 = val; o.p249 = val;
  o.p250 = val; o.p251 = val; o.p252 = val; o.p253 = val; o.p254 = val;
  // Repeat p254 on o2; they should share the same cache index (254).
  o2.p254 = val;
  // Further properties should use the overflow index (255).
  o.p255 = val;
  o.p256 = val;
  o.p257 = val;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 261
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 3
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  StringSwitchImm count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:i0[ASCII, 0..11] #9335CE8A: manyGetByIds
// CHECK-NEXT:i1[ASCII, 12..23] #1E6A92DA: manyPutByIds
// CHECK-NEXT:i2[ASCII, 24..25] #073CCA8F: p0
// CHECK-NEXT:i3[ASCII, 26..27] #073CCE9C: p1
// CHECK-NEXT:i4[ASCII, 26..28] #F99E2137: p10
// CHECK-NEXT:i5[ASCII, 26..29] #73EB3392: p100
// CHECK-NEXT:i6[ASCII, 30..33] #73EB4E6D: p101
// CHECK-NEXT:i7[ASCII, 34..37] #73EB4A7C: p102
// CHECK-NEXT:i8[ASCII, 38..41] #73EB464F: p103
// CHECK-NEXT:i9[ASCII, 42..45] #73EB425E: p104
// CHECK-NEXT:i10[ASCII, 46..49] #73EB5E29: p105
// CHECK-NEXT:i11[ASCII, 50..53] #73EB5A38: p106
// CHECK-NEXT:i12[ASCII, 54..57] #73EB560B: p107
// CHECK-NEXT:i13[ASCII, 58..61] #73EB521A: p108
// CHECK-NEXT:i14[ASCII, 62..65] #73EB6EF5: p109
// CHECK-NEXT:i15[ASCII, 66..68] #F991E2C6: p11
// CHECK-NEXT:i16[ASCII, 66..69] #4019CC1D: p110
// CHECK-NEXT:i17[ASCII, 70..73] #4019C80C: p111
// CHECK-NEXT:i18[ASCII, 74..77] #4019B5F3: p112
// CHECK-NEXT:i19[ASCII, 78..81] #4019B1E2: p113
// CHECK-NEXT:i20[ASCII, 82..85] #4019BDD1: p114
// CHECK-NEXT:i21[ASCII, 86..89] #4019B9C0: p115
// CHECK-NEXT:i22[ASCII, 90..93] #4019A5B7: p116
// CHECK-NEXT:i23[ASCII, 94..97] #4019A1A6: p117
// CHECK-NEXT:i24[ASCII, 98..101] #4019AD95: p118
// CHECK-NEXT:i25[ASCII, 102..105] #4019A984: p119
// CHECK-NEXT:i26[ASCII, 106..108] #F991E6D5: p12
// CHECK-NEXT:i27[ASCII, 106..109] #40294CE9: p120
// CHECK-NEXT:i28[ASCII, 110..113] #402948FA: p121
// CHECK-NEXT:i29[ASCII, 114..117] #402ABB0B: p122
// CHECK-NEXT:i30[ASCII, 118..121] #402ABF14: p123
// CHECK-NEXT:i31[ASCII, 122..125] #402AB325: p124
// CHECK-NEXT:i32[ASCII, 126..129] #402AB736: p125
// CHECK-NEXT:i33[ASCII, 130..133] #402AAB47: p126
// CHECK-NEXT:i34[ASCII, 134..137] #402AAF50: p127
// CHECK-NEXT:i35[ASCII, 138..141] #402AA361: p128
// CHECK-NEXT:i36[ASCII, 142..145] #402AA772: p129
// CHECK-NEXT:i37[ASCII, 146..148] #F991EAE4: p13
// CHECK-NEXT:i38[ASCII, 146..149] #403AC3F8: p130
// CHECK-NEXT:i39[ASCII, 150..153] #403AC7E9: p131
// CHECK-NEXT:i40[ASCII, 154..157] #403ABA1A: p132
// CHECK-NEXT:i41[ASCII, 158..161] #403ABE0B: p133
// CHECK-NEXT:i42[ASCII, 162..165] #403AB234: p134
// CHECK-NEXT:i43[ASCII, 166..169] #403AB625: p135
// CHECK-NEXT:i44[ASCII, 170..173] #403AAA56: p136
// CHECK-NEXT:i45[ASCII, 174..177] #403AAE47: p137
// CHECK-NEXT:i46[ASCII, 178..181] #403AA270: p138
// CHECK-NEXT:i47[ASCII, 182..185] #403AA661: p139
// CHECK-NEXT:i48[ASCII, 186..188] #F991EEEB: p14
// CHECK-NEXT:i49[ASCII, 186..189] #404B6277: p140
// CHECK-NEXT:i50[ASCII, 190..193] #404B6660: p141
// CHECK-NEXT:i51[ASCII, 194..197] #404B5A91: p142
// CHECK-NEXT:i52[ASCII, 198..201] #404B5E82: p143
// CHECK-NEXT:i53[ASCII, 202..205] #404B52B3: p144
// CHECK-NEXT:i54[ASCII, 206..209] #404B569C: p145
// CHECK-NEXT:i55[ASCII, 210..213] #404B4AED: p146
// CHECK-NEXT:i56[ASCII, 214..217] #404B4EFE: p147
// CHECK-NEXT:i57[ASCII, 218..221] #404B42CF: p148
// CHECK-NEXT:i58[ASCII, 222..225] #404B46D8: p149
// CHECK-NEXT:i59[ASCII, 226..228] #F991F29A: p15
// CHECK-NEXT:i60[ASCII, 226..229] #40586EA1: p150
// CHECK-NEXT:i61[ASCII, 230..233] #40586AB0: p151
// CHECK-NEXT:i62[ASCII, 234..237] #40585647: p152
// CHECK-NEXT:i63[ASCII, 238..241] #40585256: p153
// CHECK-NEXT:i64[ASCII, 242..245] #40585E65: p154
// CHECK-NEXT:i65[ASCII, 246..249] #40585A74: p155
// CHECK-NEXT:i66[ASCII, 250..253] #4058461B: p156
// CHECK-NEXT:i67[ASCII, 254..257] #4058420A: p157
// CHECK-NEXT:i68[ASCII, 258..261] #40584E39: p158
// CHECK-NEXT:i69[ASCII, 262..265] #40584A28: p159
// CHECK-NEXT:i70[ASCII, 266..268] #F991F689: p16
// CHECK-NEXT:i71[ASCII, 266..269] #406969D3: p160
// CHECK-NEXT:i72[ASCII, 270..273] #40696DC0: p161
// CHECK-NEXT:i73[ASCII, 274..277] #40695131: p162
// CHECK-NEXT:i74[ASCII, 278..281] #40695526: p163
// CHECK-NEXT:i75[ASCII, 282..285] #40695917: p164
// CHECK-NEXT:i76[ASCII, 286..289] #40695D04: p165
// CHECK-NEXT:i77[ASCII, 290..293] #40694175: p166
// CHECK-NEXT:i78[ASCII, 294..297] #4069451B: p167
// CHECK-NEXT:i79[ASCII, 298..301] #4069492A: p168
// CHECK-NEXT:i80[ASCII, 302..305] #40694D39: p169
// CHECK-NEXT:i81[ASCII, 306..308] #F991FAB8: p17
// CHECK-NEXT:i82[ASCII, 306..309] #40786C83: p170
// CHECK-NEXT:i83[ASCII, 310..313] #40786892: p171
// CHECK-NEXT:i84[ASCII, 314..317] #40785461: p172
// CHECK-NEXT:i85[ASCII, 318..321] #40785070: p173
// CHECK-NEXT:i86[ASCII, 322..325] #40785C47: p174
// CHECK-NEXT:i87[ASCII, 326..329] #40785856: p175
// CHECK-NEXT:i88[ASCII, 330..333] #40784425: p176
// CHECK-NEXT:i89[ASCII, 334..337] #40784034: p177
// CHECK-NEXT:i90[ASCII, 338..341] #40784C1B: p178
// CHECK-NEXT:i91[ASCII, 342..345] #4078480A: p179
// CHECK-NEXT:i92[ASCII, 346..348] #F991FEAF: p18
// CHECK-NEXT:i93[ASCII, 346..349] #408B4F34: p180
// CHECK-NEXT:i94[ASCII, 350..353] #408B4B1B: p181
// CHECK-NEXT:i95[ASCII, 354..357] #408BB4EA: p182
// CHECK-NEXT:i96[ASCII, 358..361] #408BB0F9: p183
// CHECK-NEXT:i97[ASCII, 362..365] #408BBCC8: p184
// CHECK-NEXT:i98[ASCII, 366..369] #408BB8DF: p185
// CHECK-NEXT:i99[ASCII, 370..373] #408BA4AE: p186
// CHECK-NEXT:i100[ASCII, 374..377] #408BA0BD: p187
// CHECK-NEXT:i101[ASCII, 378..381] #408BAC8C: p188
// CHECK-NEXT:i102[ASCII, 382..385] #408BA893: p189
// CHECK-NEXT:i103[ASCII, 386..388] #F991C25E: p19
// CHECK-NEXT:i104[ASCII, 386..389] #41999564: p190
// CHECK-NEXT:i105[ASCII, 390..393] #41999175: p191
// CHECK-NEXT:i106[ASCII, 394..397] #419E729A: p192
// CHECK-NEXT:i107[ASCII, 398..401] #419E768B: p193
// CHECK-NEXT:i108[ASCII, 402..405] #419E7AB8: p194
// CHECK-NEXT:i109[ASCII, 406..409] #419E7EA9: p195
// CHECK-NEXT:i110[ASCII, 410..413] #419E62DE: p196
// CHECK-NEXT:i111[ASCII, 414..417] #419E66CF: p197
// CHECK-NEXT:i112[ASCII, 418..421] #419E6AFC: p198
// CHECK-NEXT:i113[ASCII, 422..425] #419E6EED: p199
// CHECK-NEXT:i114[ASCII, 426..427] #073CD2ED: p2
// CHECK-NEXT:i115[ASCII, 426..428] #F9636201: p20
// CHECK-NEXT:i116[ASCII, 426..429] #84F796A9: p200
// CHECK-NEXT:i117[ASCII, 430..433] #84F79A9A: p201
// CHECK-NEXT:i118[ASCII, 434..437] #84F79E8B: p202
// CHECK-NEXT:i119[ASCII, 438..441] #84F782FC: p203
// CHECK-NEXT:i120[ASCII, 442..445] #84F786ED: p204
// CHECK-NEXT:i121[ASCII, 446..449] #84F78ADE: p205
// CHECK-NEXT:i122[ASCII, 450..453] #84F78ECF: p206
// CHECK-NEXT:i123[ASCII, 454..457] #84F7F330: p207
// CHECK-NEXT:i124[ASCII, 458..461] #84F7F721: p208
// CHECK-NEXT:i125[ASCII, 462..465] #84F7FB12: p209
// CHECK-NEXT:i126[ASCII, 466..468] #F9636E32: p21
// CHECK-NEXT:i127[ASCII, 466..469] #850085BB: p210
// CHECK-NEXT:i128[ASCII, 470..473] #8500898A: p211
// CHECK-NEXT:i129[ASCII, 474..477] #85008D9D: p212
// CHECK-NEXT:i130[ASCII, 478..481] #8501766C: p213
// CHECK-NEXT:i131[ASCII, 482..485] #8501727F: p214
// CHECK-NEXT:i132[ASCII, 486..489] #85017E4E: p215
// CHECK-NEXT:i133[ASCII, 490..493] #85017A51: p216
// CHECK-NEXT:i134[ASCII, 494..497] #85016620: p217
// CHECK-NEXT:i135[ASCII, 498..501] #85016233: p218
// CHECK-NEXT:i136[ASCII, 502..505] #85016E02: p219
// CHECK-NEXT:i137[ASCII, 506..508] #F9636A23: p22
// CHECK-NEXT:i138[ASCII, 506..509] #8510848A: p220
// CHECK-NEXT:i139[ASCII, 510..513] #851088BD: p221
// CHECK-NEXT:i140[ASCII, 514..517] #85108CAC: p222
// CHECK-NEXT:i141[ASCII, 518..521] #8510F15F: p223
// CHECK-NEXT:i142[ASCII, 522..525] #8510F54E: p224
// CHECK-NEXT:i143[ASCII, 526..529] #8510F971: p225
// CHECK-NEXT:i144[ASCII, 530..533] #8510FD60: p226
// CHECK-NEXT:i145[ASCII, 534..537] #8510E113: p227
// CHECK-NEXT:i146[ASCII, 538..541] #8510E502: p228
// CHECK-NEXT:i147[ASCII, 542..545] #8510E935: p229
// CHECK-NEXT:i148[ASCII, 546..548] #F963766C: p23
// CHECK-NEXT:i149[ASCII, 546..549] #85211106: p230
// CHECK-NEXT:i150[ASCII, 550..553] #85211D37: p231
// CHECK-NEXT:i151[ASCII, 554..557] #85211924: p232
// CHECK-NEXT:i152[ASCII, 558..561] #85210555: p233
// CHECK-NEXT:i153[ASCII, 562..565] #8521017A: p234
// CHECK-NEXT:i154[ASCII, 566..569] #85210D4B: p235
// CHECK-NEXT:i155[ASCII, 570..573] #85210958: p236
// CHECK-NEXT:i156[ASCII, 574..577] #8522FAA9: p237
// CHECK-NEXT:i157[ASCII, 578..581] #8522FEBE: p238
// CHECK-NEXT:i158[ASCII, 582..585] #8522F28F: p239
// CHECK-NEXT:i159[ASCII, 586..588] #F963727D: p24
// CHECK-NEXT:i160[ASCII, 586..589] #85329E37: p240
// CHECK-NEXT:i161[ASCII, 590..593] #85329204: p241
// CHECK-NEXT:i162[ASCII, 594..597] #85329615: p242
// CHECK-NEXT:i163[ASCII, 598..601] #85328A7A: p243
// CHECK-NEXT:i164[ASCII, 602..605] #85328E6B: p244
// CHECK-NEXT:i165[ASCII, 606..609] #85328258: p245
// CHECK-NEXT:i166[ASCII, 610..613] #85328649: p246
// CHECK-NEXT:i167[ASCII, 614..617] #8532FBBE: p247
// CHECK-NEXT:i168[ASCII, 618..621] #8532FFAF: p248
// CHECK-NEXT:i169[ASCII, 622..625] #8532F39C: p249
// CHECK-NEXT:i170[ASCII, 626..628] #F9637E4E: p25
// CHECK-NEXT:i171[ASCII, 626..629] #854003A7: p250
// CHECK-NEXT:i172[ASCII, 630..633] #85400F96: p251
// CHECK-NEXT:i173[ASCII, 634..637] #8540F098: p254
// CHECK-NEXT:i174[ASCII, 638..640] #F9637A5F: p26
// CHECK-NEXT:i175[ASCII, 641..643] #F96346A8: p27
// CHECK-NEXT:i176[ASCII, 644..646] #F96342B9: p28
// CHECK-NEXT:i177[ASCII, 647..649] #F9634E8A: p29
// CHECK-NEXT:i178[ASCII, 650..651] #073CD6FA: p3
// CHECK-NEXT:i179[ASCII, 650..652] #F9731AD6: p30
// CHECK-NEXT:i180[ASCII, 653..655] #F973E527: p31
// CHECK-NEXT:i181[ASCII, 656..658] #F973E130: p32
// CHECK-NEXT:i182[ASCII, 659..661] #F973ED01: p33
// CHECK-NEXT:i183[ASCII, 662..664] #F973E912: p34
// CHECK-NEXT:i184[ASCII, 665..667] #F973F563: p35
// CHECK-NEXT:i185[ASCII, 668..670] #F973F16C: p36
// CHECK-NEXT:i186[ASCII, 671..673] #F973FD5D: p37
// CHECK-NEXT:i187[ASCII, 674..676] #F973F94E: p38
// CHECK-NEXT:i188[ASCII, 677..679] #F973C5BF: p39
// CHECK-NEXT:i189[ASCII, 680..681] #073CDACB: p4
// CHECK-NEXT:i190[ASCII, 680..682] #F94265E0: p40
// CHECK-NEXT:i191[ASCII, 683..685] #F94269D7: p41
// CHECK-NEXT:i192[ASCII, 686..688] #F9426DC6: p42
// CHECK-NEXT:i193[ASCII, 689..691] #F94271B5: p43
// CHECK-NEXT:i194[ASCII, 692..694] #F94275A4: p44
// CHECK-NEXT:i195[ASCII, 695..697] #F942786C: p45
// CHECK-NEXT:i196[ASCII, 698..700] #F9427C7D: p46
// CHECK-NEXT:i197[ASCII, 701..703] #F942408E: p47
// CHECK-NEXT:i198[ASCII, 704..706] #F942449F: p48
// CHECK-NEXT:i199[ASCII, 707..709] #F94248A8: p49
// CHECK-NEXT:i200[ASCII, 710..711] #073CDED8: p5
// CHECK-NEXT:i201[ASCII, 710..712] #F9521CF4: p50
// CHECK-NEXT:i202[ASCII, 713..715] #F953E705: p51
// CHECK-NEXT:i203[ASCII, 716..718] #F953E316: p52
// CHECK-NEXT:i204[ASCII, 719..721] #F953EF27: p53
// CHECK-NEXT:i205[ASCII, 722..724] #F953EB30: p54
// CHECK-NEXT:i206[ASCII, 725..727] #F953F741: p55
// CHECK-NEXT:i207[ASCII, 728..730] #F953F352: p56
// CHECK-NEXT:i208[ASCII, 731..733] #F953FF63: p57
// CHECK-NEXT:i209[ASCII, 734..736] #F953FB6C: p58
// CHECK-NEXT:i210[ASCII, 737..739] #F953C79D: p59
// CHECK-NEXT:i211[ASCII, 740..741] #073CE229: p6
// CHECK-NEXT:i212[ASCII, 740..742] #F92D5F40: p60
// CHECK-NEXT:i213[ASCII, 743..745] #F92D5373: p61
// CHECK-NEXT:i214[ASCII, 746..748] #F92D5762: p62
// CHECK-NEXT:i215[ASCII, 749..751] #F92D4B15: p63
// CHECK-NEXT:i216[ASCII, 752..754] #F92D4F04: p64
// CHECK-NEXT:i217[ASCII, 755..757] #F92D4337: p65
// CHECK-NEXT:i218[ASCII, 758..760] #F92D4726: p66
// CHECK-NEXT:i219[ASCII, 761..763] #F92D7BE9: p67
// CHECK-NEXT:i220[ASCII, 764..766] #F92D7FF8: p68
// CHECK-NEXT:i221[ASCII, 767..769] #F92D73CB: p69
// CHECK-NEXT:i222[ASCII, 770..771] #073CE636: p7
// CHECK-NEXT:i223[ASCII, 770..772] #F93D279F: p70
// CHECK-NEXT:i224[ASCII, 773..775] #F93DD86E: p71
// CHECK-NEXT:i225[ASCII, 776..778] #F93DDC71: p72
// CHECK-NEXT:i226[ASCII, 779..781] #F93DD040: p73
// CHECK-NEXT:i227[ASCII, 782..784] #F93DD453: p74
// CHECK-NEXT:i228[ASCII, 785..787] #F93DC822: p75
// CHECK-NEXT:i229[ASCII, 788..790] #F93DCC35: p76
// CHECK-NEXT:i230[ASCII, 791..793] #F93DC004: p77
// CHECK-NEXT:i231[ASCII, 794..796] #F93DC417: p78
// CHECK-NEXT:i232[ASCII, 797..799] #F93DF8E6: p79
// CHECK-NEXT:i233[ASCII, 800..801] #073CEA07: p8
// CHECK-NEXT:i234[ASCII, 800..802] #F90E512F: p80
// CHECK-NEXT:i235[ASCII, 803..805] #F90E5D10: p81
// CHECK-NEXT:i236[ASCII, 806..808] #F90E5901: p82
// CHECK-NEXT:i237[ASCII, 809..811] #F90E4572: p83
// CHECK-NEXT:i238[ASCII, 812..814] #F90E4163: p84
// CHECK-NEXT:i239[ASCII, 815..817] #F90E4D54: p85
// CHECK-NEXT:i240[ASCII, 818..820] #F90E4945: p86
// CHECK-NEXT:i241[ASCII, 821..823] #F90E75B6: p87
// CHECK-NEXT:i242[ASCII, 824..826] #F90E71A7: p88
// CHECK-NEXT:i243[ASCII, 827..829] #F90E7DE9: p89
// CHECK-NEXT:i244[ASCII, 830..831] #073CEE14: p9
// CHECK-NEXT:i245[ASCII, 830..832] #F91E29BD: p90
// CHECK-NEXT:i246[ASCII, 833..835] #F91DDA4C: p91
// CHECK-NEXT:i247[ASCII, 836..838] #F91DDE5F: p92
// CHECK-NEXT:i248[ASCII, 839..841] #F91DD26E: p93
// CHECK-NEXT:i249[ASCII, 842..844] #F91DD671: p94
// CHECK-NEXT:i250[ASCII, 845..847] #F91DCA00: p95
// CHECK-NEXT:i251[ASCII, 848..850] #F91DCE13: p96
// CHECK-NEXT:i252[ASCII, 851..853] #F91DC222: p97
// CHECK-NEXT:i253[ASCII, 854..856] #F91DC635: p98
// CHECK-NEXT:i254[ASCII, 857..859] #F91DFAC4: p99
// CHECK-NEXT:s255[ASCII, 860..865]: global
// CHECK-NEXT:i256[ASCII, 866..869] #85400B7A: p252
// CHECK-NEXT:i257[ASCII, 870..873] #8540F48B: p253
// CHECK-NEXT:i258[ASCII, 874..877] #8540FCA9: p255
// CHECK-NEXT:i259[ASCII, 878..881] #8540F8BE: p256
// CHECK-NEXT:i260[ASCII, 882..885] #8540E4CF: p257

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "manyGetByIds"
// CHECK-NEXT:    DeclareGlobalVar  "manyPutByIds"
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<manyGetByIds>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "manyGetByIds"
// CHECK-NEXT:    CreateClosure     r1, r0, Function<manyPutByIds>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "manyPutByIds"
// CHECK-NEXT:    Ret               r0

// CHECK:Function<manyGetByIds>(3 params, 4 registers, 1 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0011
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    GetByIdShort      r3, r1, 0, "p0"
// CHECK-NEXT:    GetByIdShort      r2, r1, 1, "p1"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 2, "p2"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 3, "p3"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 4, "p4"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 5, "p5"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 6, "p6"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 7, "p7"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 8, "p8"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 9, "p9"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 10, "p10"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 11, "p11"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 12, "p12"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 13, "p13"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 14, "p14"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 15, "p15"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 16, "p16"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 17, "p17"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 18, "p18"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 19, "p19"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 20, "p20"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 21, "p21"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 22, "p22"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 23, "p23"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 24, "p24"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 25, "p25"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 26, "p26"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 27, "p27"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 28, "p28"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 29, "p29"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 30, "p30"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 31, "p31"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 32, "p32"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 33, "p33"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 34, "p34"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 35, "p35"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 36, "p36"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 37, "p37"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 38, "p38"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 39, "p39"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 40, "p40"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 41, "p41"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 42, "p42"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 43, "p43"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 44, "p44"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 45, "p45"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 46, "p46"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 47, "p47"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 48, "p48"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 49, "p49"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 50, "p50"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 51, "p51"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 52, "p52"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 53, "p53"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 54, "p54"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 55, "p55"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 56, "p56"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 57, "p57"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 58, "p58"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 59, "p59"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 60, "p60"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 61, "p61"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 62, "p62"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 63, "p63"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 64, "p64"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 65, "p65"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 66, "p66"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 67, "p67"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 68, "p68"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 69, "p69"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 70, "p70"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 71, "p71"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 72, "p72"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 73, "p73"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 74, "p74"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 75, "p75"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 76, "p76"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 77, "p77"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 78, "p78"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 79, "p79"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 80, "p80"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 81, "p81"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 82, "p82"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 83, "p83"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 84, "p84"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 85, "p85"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 86, "p86"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 87, "p87"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 88, "p88"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 89, "p89"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 90, "p90"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 91, "p91"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 92, "p92"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 93, "p93"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 94, "p94"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 95, "p95"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 96, "p96"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 97, "p97"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 98, "p98"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 99, "p99"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 100, "p100"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 101, "p101"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 102, "p102"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 103, "p103"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 104, "p104"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 105, "p105"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 106, "p106"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 107, "p107"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 108, "p108"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 109, "p109"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 110, "p110"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 111, "p111"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 112, "p112"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 113, "p113"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 114, "p114"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 115, "p115"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 116, "p116"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 117, "p117"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 118, "p118"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 119, "p119"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 120, "p120"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 121, "p121"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 122, "p122"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 123, "p123"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 124, "p124"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 125, "p125"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 126, "p126"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 127, "p127"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 128, "p128"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 129, "p129"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 130, "p130"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 131, "p131"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 132, "p132"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 133, "p133"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 134, "p134"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 135, "p135"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 136, "p136"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 137, "p137"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 138, "p138"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 139, "p139"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 140, "p140"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 141, "p141"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 142, "p142"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 143, "p143"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 144, "p144"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 145, "p145"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 146, "p146"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 147, "p147"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 148, "p148"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 149, "p149"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 150, "p150"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 151, "p151"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 152, "p152"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 153, "p153"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 154, "p154"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 155, "p155"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 156, "p156"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 157, "p157"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 158, "p158"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 159, "p159"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 160, "p160"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 161, "p161"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 162, "p162"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 163, "p163"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 164, "p164"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 165, "p165"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 166, "p166"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 167, "p167"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 168, "p168"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 169, "p169"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 170, "p170"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 171, "p171"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 172, "p172"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 173, "p173"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 174, "p174"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 175, "p175"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 176, "p176"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 177, "p177"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 178, "p178"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 179, "p179"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 180, "p180"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 181, "p181"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 182, "p182"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 183, "p183"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 184, "p184"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 185, "p185"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 186, "p186"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 187, "p187"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 188, "p188"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 189, "p189"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 190, "p190"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 191, "p191"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 192, "p192"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 193, "p193"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 194, "p194"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 195, "p195"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 196, "p196"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 197, "p197"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 198, "p198"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 199, "p199"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 200, "p200"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 201, "p201"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 202, "p202"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 203, "p203"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 204, "p204"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 205, "p205"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 206, "p206"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 207, "p207"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 208, "p208"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 209, "p209"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 210, "p210"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 211, "p211"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 212, "p212"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 213, "p213"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 214, "p214"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 215, "p215"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 216, "p216"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 217, "p217"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 218, "p218"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 219, "p219"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 220, "p220"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 221, "p221"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 222, "p222"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 223, "p223"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 224, "p224"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 225, "p225"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 226, "p226"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 227, "p227"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 228, "p228"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 229, "p229"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 230, "p230"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 231, "p231"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 232, "p232"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 233, "p233"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 234, "p234"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 235, "p235"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 236, "p236"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 237, "p237"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 238, "p238"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 239, "p239"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 240, "p240"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 241, "p241"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 242, "p242"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 243, "p243"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 244, "p244"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 245, "p245"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 246, "p246"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 247, "p247"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 248, "p248"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 249, "p249"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 250, "p250"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 251, "p251"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetById           r2, r1, 252, "p252"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetById           r2, r1, 253, "p253"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetByIdShort      r2, r1, 254, "p254"
// CHECK-NEXT:    Add               r2, r3, r2
// CHECK-NEXT:    LoadConstZero     r0
// CHECK-NEXT:    Add               r3, r0, r2
// CHECK-NEXT:    LoadParam         r2, 2
// CHECK-NEXT:    GetByIdShort      r2, r2, 254, "p254"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetById           r2, r1, 255, "p255"
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    GetById           r2, r1, 255, "p256"
// CHECK-NEXT:    Add               r2, r3, r2
// CHECK-NEXT:    GetById           r1, r1, 255, "p257"
// CHECK-NEXT:    Add               r1, r2, r1
// CHECK-NEXT:    Ret               r1

// CHECK:Function<manyPutByIds>(4 params, 4 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x07a2
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:    LoadParam         r1, 3
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "p0"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "p1"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 2, "p2"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 3, "p3"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 4, "p4"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 5, "p5"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 6, "p6"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 7, "p7"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 8, "p8"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 9, "p9"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 10, "p10"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 11, "p11"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 12, "p12"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 13, "p13"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 14, "p14"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 15, "p15"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 16, "p16"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 17, "p17"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 18, "p18"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 19, "p19"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 20, "p20"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 21, "p21"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 22, "p22"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 23, "p23"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 24, "p24"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 25, "p25"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 26, "p26"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 27, "p27"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 28, "p28"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 29, "p29"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 30, "p30"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 31, "p31"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 32, "p32"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 33, "p33"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 34, "p34"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 35, "p35"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 36, "p36"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 37, "p37"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 38, "p38"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 39, "p39"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 40, "p40"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 41, "p41"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 42, "p42"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 43, "p43"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 44, "p44"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 45, "p45"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 46, "p46"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 47, "p47"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 48, "p48"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 49, "p49"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 50, "p50"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 51, "p51"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 52, "p52"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 53, "p53"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 54, "p54"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 55, "p55"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 56, "p56"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 57, "p57"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 58, "p58"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 59, "p59"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 60, "p60"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 61, "p61"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 62, "p62"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 63, "p63"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 64, "p64"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 65, "p65"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 66, "p66"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 67, "p67"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 68, "p68"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 69, "p69"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 70, "p70"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 71, "p71"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 72, "p72"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 73, "p73"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 74, "p74"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 75, "p75"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 76, "p76"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 77, "p77"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 78, "p78"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 79, "p79"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 80, "p80"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 81, "p81"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 82, "p82"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 83, "p83"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 84, "p84"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 85, "p85"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 86, "p86"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 87, "p87"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 88, "p88"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 89, "p89"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 90, "p90"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 91, "p91"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 92, "p92"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 93, "p93"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 94, "p94"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 95, "p95"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 96, "p96"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 97, "p97"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 98, "p98"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 99, "p99"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 100, "p100"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 101, "p101"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 102, "p102"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 103, "p103"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 104, "p104"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 105, "p105"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 106, "p106"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 107, "p107"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 108, "p108"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 109, "p109"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 110, "p110"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 111, "p111"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 112, "p112"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 113, "p113"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 114, "p114"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 115, "p115"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 116, "p116"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 117, "p117"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 118, "p118"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 119, "p119"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 120, "p120"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 121, "p121"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 122, "p122"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 123, "p123"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 124, "p124"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 125, "p125"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 126, "p126"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 127, "p127"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 128, "p128"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 129, "p129"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 130, "p130"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 131, "p131"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 132, "p132"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 133, "p133"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 134, "p134"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 135, "p135"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 136, "p136"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 137, "p137"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 138, "p138"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 139, "p139"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 140, "p140"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 141, "p141"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 142, "p142"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 143, "p143"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 144, "p144"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 145, "p145"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 146, "p146"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 147, "p147"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 148, "p148"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 149, "p149"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 150, "p150"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 151, "p151"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 152, "p152"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 153, "p153"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 154, "p154"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 155, "p155"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 156, "p156"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 157, "p157"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 158, "p158"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 159, "p159"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 160, "p160"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 161, "p161"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 162, "p162"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 163, "p163"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 164, "p164"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 165, "p165"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 166, "p166"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 167, "p167"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 168, "p168"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 169, "p169"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 170, "p170"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 171, "p171"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 172, "p172"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 173, "p173"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 174, "p174"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 175, "p175"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 176, "p176"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 177, "p177"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 178, "p178"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 179, "p179"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 180, "p180"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 181, "p181"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 182, "p182"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 183, "p183"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 184, "p184"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 185, "p185"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 186, "p186"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 187, "p187"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 188, "p188"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 189, "p189"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 190, "p190"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 191, "p191"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 192, "p192"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 193, "p193"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 194, "p194"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 195, "p195"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 196, "p196"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 197, "p197"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 198, "p198"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 199, "p199"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 200, "p200"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 201, "p201"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 202, "p202"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 203, "p203"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 204, "p204"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 205, "p205"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 206, "p206"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 207, "p207"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 208, "p208"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 209, "p209"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 210, "p210"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 211, "p211"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 212, "p212"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 213, "p213"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 214, "p214"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 215, "p215"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 216, "p216"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 217, "p217"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 218, "p218"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 219, "p219"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 220, "p220"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 221, "p221"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 222, "p222"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 223, "p223"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 224, "p224"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 225, "p225"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 226, "p226"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 227, "p227"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 228, "p228"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 229, "p229"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 230, "p230"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 231, "p231"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 232, "p232"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 233, "p233"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 234, "p234"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 235, "p235"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 236, "p236"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 237, "p237"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 238, "p238"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 239, "p239"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 240, "p240"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 241, "p241"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 242, "p242"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 243, "p243"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 244, "p244"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 245, "p245"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 246, "p246"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 247, "p247"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 248, "p248"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 249, "p249"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 250, "p250"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 251, "p251"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 252, "p252"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 253, "p253"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 254, "p254"
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    PutByIdLoose      r3, r1, 254, "p254"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 255, "p255"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 255, "p256"
// CHECK-NEXT:    PutByIdLoose      r2, r1, 255, "p257"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}inline-cache-ids.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 5: line 10 col 1
// CHECK-NEXT:    bc 19: line 10 col 1
// CHECK-NEXT:    bc 30: line 10 col 1
// CHECK-NEXT:  0x0011  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 3: line 14 col 6
// CHECK-NEXT:    bc 8: line 14 col 13
// CHECK-NEXT:    bc 13: line 14 col 5
// CHECK-NEXT:    bc 17: line 14 col 20
// CHECK-NEXT:    bc 22: line 14 col 5
// CHECK-NEXT:    bc 26: line 14 col 27
// CHECK-NEXT:    bc 31: line 14 col 5
// CHECK-NEXT:    bc 35: line 14 col 34
// CHECK-NEXT:    bc 40: line 14 col 5
// CHECK-NEXT:    bc 44: line 14 col 41
// CHECK-NEXT:    bc 49: line 14 col 5
// CHECK-NEXT:    bc 53: line 14 col 48
// CHECK-NEXT:    bc 58: line 14 col 5
// CHECK-NEXT:    bc 62: line 14 col 55
// CHECK-NEXT:    bc 67: line 14 col 5
// CHECK-NEXT:    bc 71: line 14 col 62
// CHECK-NEXT:    bc 76: line 14 col 5
// CHECK-NEXT:    bc 80: line 14 col 69
// CHECK-NEXT:    bc 85: line 14 col 5
// CHECK-NEXT:    bc 89: line 15 col 6
// CHECK-NEXT:    bc 94: line 14 col 5
// CHECK-NEXT:    bc 98: line 15 col 14
// CHECK-NEXT:    bc 103: line 14 col 5
// CHECK-NEXT:    bc 107: line 15 col 22
// CHECK-NEXT:    bc 112: line 14 col 5
// CHECK-NEXT:    bc 116: line 15 col 30
// CHECK-NEXT:    bc 121: line 14 col 5
// CHECK-NEXT:    bc 125: line 15 col 38
// CHECK-NEXT:    bc 130: line 14 col 5
// CHECK-NEXT:    bc 134: line 15 col 46
// CHECK-NEXT:    bc 139: line 14 col 5
// CHECK-NEXT:    bc 143: line 15 col 54
// CHECK-NEXT:    bc 148: line 14 col 5
// CHECK-NEXT:    bc 152: line 15 col 62
// CHECK-NEXT:    bc 157: line 14 col 5
// CHECK-NEXT:    bc 161: line 15 col 70
// CHECK-NEXT:    bc 166: line 14 col 5
// CHECK-NEXT:    bc 170: line 15 col 78
// CHECK-NEXT:    bc 175: line 14 col 5
// CHECK-NEXT:    bc 179: line 16 col 6
// CHECK-NEXT:    bc 184: line 14 col 5
// CHECK-NEXT:    bc 188: line 16 col 14
// CHECK-NEXT:    bc 193: line 14 col 5
// CHECK-NEXT:    bc 197: line 16 col 22
// CHECK-NEXT:    bc 202: line 14 col 5
// CHECK-NEXT:    bc 206: line 16 col 30
// CHECK-NEXT:    bc 211: line 14 col 5
// CHECK-NEXT:    bc 215: line 16 col 38
// CHECK-NEXT:    bc 220: line 14 col 5
// CHECK-NEXT:    bc 224: line 16 col 46
// CHECK-NEXT:    bc 229: line 14 col 5
// CHECK-NEXT:    bc 233: line 16 col 54
// CHECK-NEXT:    bc 238: line 14 col 5
// CHECK-NEXT:    bc 242: line 16 col 62
// CHECK-NEXT:    bc 247: line 14 col 5
// CHECK-NEXT:    bc 251: line 16 col 70
// CHECK-NEXT:    bc 256: line 14 col 5
// CHECK-NEXT:    bc 260: line 16 col 78
// CHECK-NEXT:    bc 265: line 14 col 5
// CHECK-NEXT:    bc 269: line 17 col 6
// CHECK-NEXT:    bc 274: line 14 col 5
// CHECK-NEXT:    bc 278: line 17 col 14
// CHECK-NEXT:    bc 283: line 14 col 5
// CHECK-NEXT:    bc 287: line 17 col 22
// CHECK-NEXT:    bc 292: line 14 col 5
// CHECK-NEXT:    bc 296: line 17 col 30
// CHECK-NEXT:    bc 301: line 14 col 5
// CHECK-NEXT:    bc 305: line 17 col 38
// CHECK-NEXT:    bc 310: line 14 col 5
// CHECK-NEXT:    bc 314: line 17 col 46
// CHECK-NEXT:    bc 319: line 14 col 5
// CHECK-NEXT:    bc 323: line 17 col 54
// CHECK-NEXT:    bc 328: line 14 col 5
// CHECK-NEXT:    bc 332: line 17 col 62
// CHECK-NEXT:    bc 337: line 14 col 5
// CHECK-NEXT:    bc 341: line 17 col 70
// CHECK-NEXT:    bc 346: line 14 col 5
// CHECK-NEXT:    bc 350: line 17 col 78
// CHECK-NEXT:    bc 355: line 14 col 5
// CHECK-NEXT:    bc 359: line 18 col 6
// CHECK-NEXT:    bc 364: line 14 col 5
// CHECK-NEXT:    bc 368: line 18 col 14
// CHECK-NEXT:    bc 373: line 14 col 5
// CHECK-NEXT:    bc 377: line 18 col 22
// CHECK-NEXT:    bc 382: line 14 col 5
// CHECK-NEXT:    bc 386: line 18 col 30
// CHECK-NEXT:    bc 391: line 14 col 5
// CHECK-NEXT:    bc 395: line 18 col 38
// CHECK-NEXT:    bc 400: line 14 col 5
// CHECK-NEXT:    bc 404: line 18 col 46
// CHECK-NEXT:    bc 409: line 14 col 5
// CHECK-NEXT:    bc 413: line 18 col 54
// CHECK-NEXT:    bc 418: line 14 col 5
// CHECK-NEXT:    bc 422: line 18 col 62
// CHECK-NEXT:    bc 427: line 14 col 5
// CHECK-NEXT:    bc 431: line 18 col 70
// CHECK-NEXT:    bc 436: line 14 col 5
// CHECK-NEXT:    bc 440: line 18 col 78
// CHECK-NEXT:    bc 445: line 14 col 5
// CHECK-NEXT:    bc 449: line 19 col 6
// CHECK-NEXT:    bc 454: line 14 col 5
// CHECK-NEXT:    bc 458: line 19 col 14
// CHECK-NEXT:    bc 463: line 14 col 5
// CHECK-NEXT:    bc 467: line 19 col 22
// CHECK-NEXT:    bc 472: line 14 col 5
// CHECK-NEXT:    bc 476: line 19 col 30
// CHECK-NEXT:    bc 481: line 14 col 5
// CHECK-NEXT:    bc 485: line 19 col 38
// CHECK-NEXT:    bc 490: line 14 col 5
// CHECK-NEXT:    bc 494: line 19 col 46
// CHECK-NEXT:    bc 499: line 14 col 5
// CHECK-NEXT:    bc 503: line 19 col 54
// CHECK-NEXT:    bc 508: line 14 col 5
// CHECK-NEXT:    bc 512: line 19 col 62
// CHECK-NEXT:    bc 517: line 14 col 5
// CHECK-NEXT:    bc 521: line 19 col 70
// CHECK-NEXT:    bc 526: line 14 col 5
// CHECK-NEXT:    bc 530: line 19 col 78
// CHECK-NEXT:    bc 535: line 14 col 5
// CHECK-NEXT:    bc 539: line 20 col 6
// CHECK-NEXT:    bc 544: line 14 col 5
// CHECK-NEXT:    bc 548: line 20 col 14
// CHECK-NEXT:    bc 553: line 14 col 5
// CHECK-NEXT:    bc 557: line 20 col 22
// CHECK-NEXT:    bc 562: line 14 col 5
// CHECK-NEXT:    bc 566: line 20 col 30
// CHECK-NEXT:    bc 571: line 14 col 5
// CHECK-NEXT:    bc 575: line 20 col 38
// CHECK-NEXT:    bc 580: line 14 col 5
// CHECK-NEXT:    bc 584: line 20 col 46
// CHECK-NEXT:    bc 589: line 14 col 5
// CHECK-NEXT:    bc 593: line 20 col 54
// CHECK-NEXT:    bc 598: line 14 col 5
// CHECK-NEXT:    bc 602: line 20 col 62
// CHECK-NEXT:    bc 607: line 14 col 5
// CHECK-NEXT:    bc 611: line 20 col 70
// CHECK-NEXT:    bc 616: line 14 col 5
// CHECK-NEXT:    bc 620: line 20 col 78
// CHECK-NEXT:    bc 625: line 14 col 5
// CHECK-NEXT:    bc 629: line 21 col 6
// CHECK-NEXT:    bc 634: line 14 col 5
// CHECK-NEXT:    bc 638: line 21 col 14
// CHECK-NEXT:    bc 643: line 14 col 5
// CHECK-NEXT:    bc 647: line 21 col 22
// CHECK-NEXT:    bc 652: line 14 col 5
// CHECK-NEXT:    bc 656: line 21 col 30
// CHECK-NEXT:    bc 661: line 14 col 5
// CHECK-NEXT:    bc 665: line 21 col 38
// CHECK-NEXT:    bc 670: line 14 col 5
// CHECK-NEXT:    bc 674: line 21 col 46
// CHECK-NEXT:    bc 679: line 14 col 5
// CHECK-NEXT:    bc 683: line 21 col 54
// CHECK-NEXT:    bc 688: line 14 col 5
// CHECK-NEXT:    bc 692: line 21 col 62
// CHECK-NEXT:    bc 697: line 14 col 5
// CHECK-NEXT:    bc 701: line 21 col 70
// CHECK-NEXT:    bc 706: line 14 col 5
// CHECK-NEXT:    bc 710: line 21 col 78
// CHECK-NEXT:    bc 715: line 14 col 5
// CHECK-NEXT:    bc 719: line 22 col 6
// CHECK-NEXT:    bc 724: line 14 col 5
// CHECK-NEXT:    bc 728: line 22 col 14
// CHECK-NEXT:    bc 733: line 14 col 5
// CHECK-NEXT:    bc 737: line 22 col 22
// CHECK-NEXT:    bc 742: line 14 col 5
// CHECK-NEXT:    bc 746: line 22 col 30
// CHECK-NEXT:    bc 751: line 14 col 5
// CHECK-NEXT:    bc 755: line 22 col 38
// CHECK-NEXT:    bc 760: line 14 col 5
// CHECK-NEXT:    bc 764: line 22 col 46
// CHECK-NEXT:    bc 769: line 14 col 5
// CHECK-NEXT:    bc 773: line 22 col 54
// CHECK-NEXT:    bc 778: line 14 col 5
// CHECK-NEXT:    bc 782: line 22 col 62
// CHECK-NEXT:    bc 787: line 14 col 5
// CHECK-NEXT:    bc 791: line 22 col 70
// CHECK-NEXT:    bc 796: line 14 col 5
// CHECK-NEXT:    bc 800: line 22 col 78
// CHECK-NEXT:    bc 805: line 14 col 5
// CHECK-NEXT:    bc 809: line 23 col 6
// CHECK-NEXT:    bc 814: line 14 col 5
// CHECK-NEXT:    bc 818: line 23 col 14
// CHECK-NEXT:    bc 823: line 14 col 5
// CHECK-NEXT:    bc 827: line 23 col 22
// CHECK-NEXT:    bc 832: line 14 col 5
// CHECK-NEXT:    bc 836: line 23 col 30
// CHECK-NEXT:    bc 841: line 14 col 5
// CHECK-NEXT:    bc 845: line 23 col 38
// CHECK-NEXT:    bc 850: line 14 col 5
// CHECK-NEXT:    bc 854: line 23 col 46
// CHECK-NEXT:    bc 859: line 14 col 5
// CHECK-NEXT:    bc 863: line 23 col 54
// CHECK-NEXT:    bc 868: line 14 col 5
// CHECK-NEXT:    bc 872: line 23 col 62
// CHECK-NEXT:    bc 877: line 14 col 5
// CHECK-NEXT:    bc 881: line 23 col 70
// CHECK-NEXT:    bc 886: line 14 col 5
// CHECK-NEXT:    bc 890: line 23 col 78
// CHECK-NEXT:    bc 895: line 14 col 5
// CHECK-NEXT:    bc 899: line 25 col 6
// CHECK-NEXT:    bc 904: line 14 col 5
// CHECK-NEXT:    bc 908: line 25 col 15
// CHECK-NEXT:    bc 913: line 14 col 5
// CHECK-NEXT:    bc 917: line 25 col 24
// CHECK-NEXT:    bc 922: line 14 col 5
// CHECK-NEXT:    bc 926: line 25 col 33
// CHECK-NEXT:    bc 931: line 14 col 5
// CHECK-NEXT:    bc 935: line 25 col 42
// CHECK-NEXT:    bc 940: line 14 col 5
// CHECK-NEXT:    bc 944: line 26 col 6
// CHECK-NEXT:    bc 949: line 14 col 5
// CHECK-NEXT:    bc 953: line 26 col 15
// CHECK-NEXT:    bc 958: line 14 col 5
// CHECK-NEXT:    bc 962: line 26 col 24
// CHECK-NEXT:    bc 967: line 14 col 5
// CHECK-NEXT:    bc 971: line 26 col 33
// CHECK-NEXT:    bc 976: line 14 col 5
// CHECK-NEXT:    bc 980: line 26 col 42
// CHECK-NEXT:    bc 985: line 14 col 5
// CHECK-NEXT:    bc 989: line 27 col 6
// CHECK-NEXT:    bc 994: line 14 col 5
// CHECK-NEXT:    bc 998: line 27 col 15
// CHECK-NEXT:    bc 1003: line 14 col 5
// CHECK-NEXT:    bc 1007: line 27 col 24
// CHECK-NEXT:    bc 1012: line 14 col 5
// CHECK-NEXT:    bc 1016: line 27 col 33
// CHECK-NEXT:    bc 1021: line 14 col 5
// CHECK-NEXT:    bc 1025: line 27 col 42
// CHECK-NEXT:    bc 1030: line 14 col 5
// CHECK-NEXT:    bc 1034: line 28 col 6
// CHECK-NEXT:    bc 1039: line 14 col 5
// CHECK-NEXT:    bc 1043: line 28 col 15
// CHECK-NEXT:    bc 1048: line 14 col 5
// CHECK-NEXT:    bc 1052: line 28 col 24
// CHECK-NEXT:    bc 1057: line 14 col 5
// CHECK-NEXT:    bc 1061: line 28 col 33
// CHECK-NEXT:    bc 1066: line 14 col 5
// CHECK-NEXT:    bc 1070: line 28 col 42
// CHECK-NEXT:    bc 1075: line 14 col 5
// CHECK-NEXT:    bc 1079: line 29 col 6
// CHECK-NEXT:    bc 1084: line 14 col 5
// CHECK-NEXT:    bc 1088: line 29 col 15
// CHECK-NEXT:    bc 1093: line 14 col 5
// CHECK-NEXT:    bc 1097: line 29 col 24
// CHECK-NEXT:    bc 1102: line 14 col 5
// CHECK-NEXT:    bc 1106: line 29 col 33
// CHECK-NEXT:    bc 1111: line 14 col 5
// CHECK-NEXT:    bc 1115: line 29 col 42
// CHECK-NEXT:    bc 1120: line 14 col 5
// CHECK-NEXT:    bc 1124: line 30 col 6
// CHECK-NEXT:    bc 1129: line 14 col 5
// CHECK-NEXT:    bc 1133: line 30 col 15
// CHECK-NEXT:    bc 1138: line 14 col 5
// CHECK-NEXT:    bc 1142: line 30 col 24
// CHECK-NEXT:    bc 1147: line 14 col 5
// CHECK-NEXT:    bc 1151: line 30 col 33
// CHECK-NEXT:    bc 1156: line 14 col 5
// CHECK-NEXT:    bc 1160: line 30 col 42
// CHECK-NEXT:    bc 1165: line 14 col 5
// CHECK-NEXT:    bc 1169: line 31 col 6
// CHECK-NEXT:    bc 1174: line 14 col 5
// CHECK-NEXT:    bc 1178: line 31 col 15
// CHECK-NEXT:    bc 1183: line 14 col 5
// CHECK-NEXT:    bc 1187: line 31 col 24
// CHECK-NEXT:    bc 1192: line 14 col 5
// CHECK-NEXT:    bc 1196: line 31 col 33
// CHECK-NEXT:    bc 1201: line 14 col 5
// CHECK-NEXT:    bc 1205: line 31 col 42
// CHECK-NEXT:    bc 1210: line 14 col 5
// CHECK-NEXT:    bc 1214: line 32 col 6
// CHECK-NEXT:    bc 1219: line 14 col 5
// CHECK-NEXT:    bc 1223: line 32 col 15
// CHECK-NEXT:    bc 1228: line 14 col 5
// CHECK-NEXT:    bc 1232: line 32 col 24
// CHECK-NEXT:    bc 1237: line 14 col 5
// CHECK-NEXT:    bc 1241: line 32 col 33
// CHECK-NEXT:    bc 1246: line 14 col 5
// CHECK-NEXT:    bc 1250: line 32 col 42
// CHECK-NEXT:    bc 1255: line 14 col 5
// CHECK-NEXT:    bc 1259: line 33 col 6
// CHECK-NEXT:    bc 1264: line 14 col 5
// CHECK-NEXT:    bc 1268: line 33 col 15
// CHECK-NEXT:    bc 1273: line 14 col 5
// CHECK-NEXT:    bc 1277: line 33 col 24
// CHECK-NEXT:    bc 1282: line 14 col 5
// CHECK-NEXT:    bc 1286: line 33 col 33
// CHECK-NEXT:    bc 1291: line 14 col 5
// CHECK-NEXT:    bc 1295: line 33 col 42
// CHECK-NEXT:    bc 1300: line 14 col 5
// CHECK-NEXT:    bc 1304: line 34 col 6
// CHECK-NEXT:    bc 1309: line 14 col 5
// CHECK-NEXT:    bc 1313: line 34 col 15
// CHECK-NEXT:    bc 1318: line 14 col 5
// CHECK-NEXT:    bc 1322: line 34 col 24
// CHECK-NEXT:    bc 1327: line 14 col 5
// CHECK-NEXT:    bc 1331: line 34 col 33
// CHECK-NEXT:    bc 1336: line 14 col 5
// CHECK-NEXT:    bc 1340: line 34 col 42
// CHECK-NEXT:    bc 1345: line 14 col 5
// CHECK-NEXT:    bc 1349: line 35 col 6
// CHECK-NEXT:    bc 1354: line 14 col 5
// CHECK-NEXT:    bc 1358: line 35 col 15
// CHECK-NEXT:    bc 1363: line 14 col 5
// CHECK-NEXT:    bc 1367: line 35 col 24
// CHECK-NEXT:    bc 1372: line 14 col 5
// CHECK-NEXT:    bc 1376: line 35 col 33
// CHECK-NEXT:    bc 1381: line 14 col 5
// CHECK-NEXT:    bc 1385: line 35 col 42
// CHECK-NEXT:    bc 1390: line 14 col 5
// CHECK-NEXT:    bc 1394: line 36 col 6
// CHECK-NEXT:    bc 1399: line 14 col 5
// CHECK-NEXT:    bc 1403: line 36 col 15
// CHECK-NEXT:    bc 1408: line 14 col 5
// CHECK-NEXT:    bc 1412: line 36 col 24
// CHECK-NEXT:    bc 1417: line 14 col 5
// CHECK-NEXT:    bc 1421: line 36 col 33
// CHECK-NEXT:    bc 1426: line 14 col 5
// CHECK-NEXT:    bc 1430: line 36 col 42
// CHECK-NEXT:    bc 1435: line 14 col 5
// CHECK-NEXT:    bc 1439: line 37 col 6
// CHECK-NEXT:    bc 1444: line 14 col 5
// CHECK-NEXT:    bc 1448: line 37 col 15
// CHECK-NEXT:    bc 1453: line 14 col 5
// CHECK-NEXT:    bc 1457: line 37 col 24
// CHECK-NEXT:    bc 1462: line 14 col 5
// CHECK-NEXT:    bc 1466: line 37 col 33
// CHECK-NEXT:    bc 1471: line 14 col 5
// CHECK-NEXT:    bc 1475: line 37 col 42
// CHECK-NEXT:    bc 1480: line 14 col 5
// CHECK-NEXT:    bc 1484: line 38 col 6
// CHECK-NEXT:    bc 1489: line 14 col 5
// CHECK-NEXT:    bc 1493: line 38 col 15
// CHECK-NEXT:    bc 1498: line 14 col 5
// CHECK-NEXT:    bc 1502: line 38 col 24
// CHECK-NEXT:    bc 1507: line 14 col 5
// CHECK-NEXT:    bc 1511: line 38 col 33
// CHECK-NEXT:    bc 1516: line 14 col 5
// CHECK-NEXT:    bc 1520: line 38 col 42
// CHECK-NEXT:    bc 1525: line 14 col 5
// CHECK-NEXT:    bc 1529: line 39 col 6
// CHECK-NEXT:    bc 1534: line 14 col 5
// CHECK-NEXT:    bc 1538: line 39 col 15
// CHECK-NEXT:    bc 1543: line 14 col 5
// CHECK-NEXT:    bc 1547: line 39 col 24
// CHECK-NEXT:    bc 1552: line 14 col 5
// CHECK-NEXT:    bc 1556: line 39 col 33
// CHECK-NEXT:    bc 1561: line 14 col 5
// CHECK-NEXT:    bc 1565: line 39 col 42
// CHECK-NEXT:    bc 1570: line 14 col 5
// CHECK-NEXT:    bc 1574: line 40 col 6
// CHECK-NEXT:    bc 1579: line 14 col 5
// CHECK-NEXT:    bc 1583: line 40 col 15
// CHECK-NEXT:    bc 1588: line 14 col 5
// CHECK-NEXT:    bc 1592: line 40 col 24
// CHECK-NEXT:    bc 1597: line 14 col 5
// CHECK-NEXT:    bc 1601: line 40 col 33
// CHECK-NEXT:    bc 1606: line 14 col 5
// CHECK-NEXT:    bc 1610: line 40 col 42
// CHECK-NEXT:    bc 1615: line 14 col 5
// CHECK-NEXT:    bc 1619: line 41 col 6
// CHECK-NEXT:    bc 1624: line 14 col 5
// CHECK-NEXT:    bc 1628: line 41 col 15
// CHECK-NEXT:    bc 1633: line 14 col 5
// CHECK-NEXT:    bc 1637: line 41 col 24
// CHECK-NEXT:    bc 1642: line 14 col 5
// CHECK-NEXT:    bc 1646: line 41 col 33
// CHECK-NEXT:    bc 1651: line 14 col 5
// CHECK-NEXT:    bc 1655: line 41 col 42
// CHECK-NEXT:    bc 1660: line 14 col 5
// CHECK-NEXT:    bc 1664: line 42 col 6
// CHECK-NEXT:    bc 1669: line 14 col 5
// CHECK-NEXT:    bc 1673: line 42 col 15
// CHECK-NEXT:    bc 1678: line 14 col 5
// CHECK-NEXT:    bc 1682: line 42 col 24
// CHECK-NEXT:    bc 1687: line 14 col 5
// CHECK-NEXT:    bc 1691: line 42 col 33
// CHECK-NEXT:    bc 1696: line 14 col 5
// CHECK-NEXT:    bc 1700: line 42 col 42
// CHECK-NEXT:    bc 1705: line 14 col 5
// CHECK-NEXT:    bc 1709: line 43 col 6
// CHECK-NEXT:    bc 1714: line 14 col 5
// CHECK-NEXT:    bc 1718: line 43 col 15
// CHECK-NEXT:    bc 1723: line 14 col 5
// CHECK-NEXT:    bc 1727: line 43 col 24
// CHECK-NEXT:    bc 1732: line 14 col 5
// CHECK-NEXT:    bc 1736: line 43 col 33
// CHECK-NEXT:    bc 1741: line 14 col 5
// CHECK-NEXT:    bc 1745: line 43 col 42
// CHECK-NEXT:    bc 1750: line 14 col 5
// CHECK-NEXT:    bc 1754: line 44 col 6
// CHECK-NEXT:    bc 1759: line 14 col 5
// CHECK-NEXT:    bc 1763: line 44 col 15
// CHECK-NEXT:    bc 1768: line 14 col 5
// CHECK-NEXT:    bc 1772: line 44 col 24
// CHECK-NEXT:    bc 1777: line 14 col 5
// CHECK-NEXT:    bc 1781: line 44 col 33
// CHECK-NEXT:    bc 1786: line 14 col 5
// CHECK-NEXT:    bc 1790: line 44 col 42
// CHECK-NEXT:    bc 1795: line 14 col 5
// CHECK-NEXT:    bc 1799: line 46 col 6
// CHECK-NEXT:    bc 1804: line 14 col 5
// CHECK-NEXT:    bc 1808: line 46 col 15
// CHECK-NEXT:    bc 1813: line 14 col 5
// CHECK-NEXT:    bc 1817: line 46 col 24
// CHECK-NEXT:    bc 1822: line 14 col 5
// CHECK-NEXT:    bc 1826: line 46 col 33
// CHECK-NEXT:    bc 1831: line 14 col 5
// CHECK-NEXT:    bc 1835: line 46 col 42
// CHECK-NEXT:    bc 1840: line 14 col 5
// CHECK-NEXT:    bc 1844: line 47 col 6
// CHECK-NEXT:    bc 1849: line 14 col 5
// CHECK-NEXT:    bc 1853: line 47 col 15
// CHECK-NEXT:    bc 1858: line 14 col 5
// CHECK-NEXT:    bc 1862: line 47 col 24
// CHECK-NEXT:    bc 1867: line 14 col 5
// CHECK-NEXT:    bc 1871: line 47 col 33
// CHECK-NEXT:    bc 1876: line 14 col 5
// CHECK-NEXT:    bc 1880: line 47 col 42
// CHECK-NEXT:    bc 1885: line 14 col 5
// CHECK-NEXT:    bc 1889: line 48 col 6
// CHECK-NEXT:    bc 1894: line 14 col 5
// CHECK-NEXT:    bc 1898: line 48 col 15
// CHECK-NEXT:    bc 1903: line 14 col 5
// CHECK-NEXT:    bc 1907: line 48 col 24
// CHECK-NEXT:    bc 1912: line 14 col 5
// CHECK-NEXT:    bc 1916: line 48 col 33
// CHECK-NEXT:    bc 1921: line 14 col 5
// CHECK-NEXT:    bc 1925: line 48 col 42
// CHECK-NEXT:    bc 1930: line 14 col 5
// CHECK-NEXT:    bc 1934: line 49 col 6
// CHECK-NEXT:    bc 1939: line 14 col 5
// CHECK-NEXT:    bc 1943: line 49 col 15
// CHECK-NEXT:    bc 1948: line 14 col 5
// CHECK-NEXT:    bc 1952: line 49 col 24
// CHECK-NEXT:    bc 1957: line 14 col 5
// CHECK-NEXT:    bc 1961: line 49 col 33
// CHECK-NEXT:    bc 1966: line 14 col 5
// CHECK-NEXT:    bc 1970: line 49 col 42
// CHECK-NEXT:    bc 1975: line 14 col 5
// CHECK-NEXT:    bc 1979: line 50 col 6
// CHECK-NEXT:    bc 1984: line 14 col 5
// CHECK-NEXT:    bc 1988: line 50 col 15
// CHECK-NEXT:    bc 1993: line 14 col 5
// CHECK-NEXT:    bc 1997: line 50 col 24
// CHECK-NEXT:    bc 2002: line 14 col 5
// CHECK-NEXT:    bc 2006: line 50 col 33
// CHECK-NEXT:    bc 2011: line 14 col 5
// CHECK-NEXT:    bc 2015: line 50 col 42
// CHECK-NEXT:    bc 2020: line 14 col 5
// CHECK-NEXT:    bc 2024: line 51 col 6
// CHECK-NEXT:    bc 2029: line 14 col 5
// CHECK-NEXT:    bc 2033: line 51 col 15
// CHECK-NEXT:    bc 2038: line 14 col 5
// CHECK-NEXT:    bc 2042: line 51 col 24
// CHECK-NEXT:    bc 2047: line 14 col 5
// CHECK-NEXT:    bc 2051: line 51 col 33
// CHECK-NEXT:    bc 2056: line 14 col 5
// CHECK-NEXT:    bc 2060: line 51 col 42
// CHECK-NEXT:    bc 2065: line 14 col 5
// CHECK-NEXT:    bc 2069: line 52 col 6
// CHECK-NEXT:    bc 2074: line 14 col 5
// CHECK-NEXT:    bc 2078: line 52 col 15
// CHECK-NEXT:    bc 2083: line 14 col 5
// CHECK-NEXT:    bc 2087: line 52 col 24
// CHECK-NEXT:    bc 2092: line 14 col 5
// CHECK-NEXT:    bc 2096: line 52 col 33
// CHECK-NEXT:    bc 2101: line 14 col 5
// CHECK-NEXT:    bc 2105: line 52 col 42
// CHECK-NEXT:    bc 2110: line 14 col 5
// CHECK-NEXT:    bc 2114: line 53 col 6
// CHECK-NEXT:    bc 2119: line 14 col 5
// CHECK-NEXT:    bc 2123: line 53 col 15
// CHECK-NEXT:    bc 2128: line 14 col 5
// CHECK-NEXT:    bc 2132: line 53 col 24
// CHECK-NEXT:    bc 2137: line 14 col 5
// CHECK-NEXT:    bc 2141: line 53 col 33
// CHECK-NEXT:    bc 2146: line 14 col 5
// CHECK-NEXT:    bc 2150: line 53 col 42
// CHECK-NEXT:    bc 2155: line 14 col 5
// CHECK-NEXT:    bc 2159: line 54 col 6
// CHECK-NEXT:    bc 2164: line 14 col 5
// CHECK-NEXT:    bc 2168: line 54 col 15
// CHECK-NEXT:    bc 2173: line 14 col 5
// CHECK-NEXT:    bc 2177: line 54 col 24
// CHECK-NEXT:    bc 2182: line 14 col 5
// CHECK-NEXT:    bc 2186: line 54 col 33
// CHECK-NEXT:    bc 2191: line 14 col 5
// CHECK-NEXT:    bc 2195: line 54 col 42
// CHECK-NEXT:    bc 2200: line 14 col 5
// CHECK-NEXT:    bc 2204: line 55 col 6
// CHECK-NEXT:    bc 2209: line 14 col 5
// CHECK-NEXT:    bc 2213: line 55 col 15
// CHECK-NEXT:    bc 2218: line 14 col 5
// CHECK-NEXT:    bc 2222: line 55 col 24
// CHECK-NEXT:    bc 2227: line 14 col 5
// CHECK-NEXT:    bc 2231: line 55 col 33
// CHECK-NEXT:    bc 2236: line 14 col 5
// CHECK-NEXT:    bc 2240: line 55 col 42
// CHECK-NEXT:    bc 2245: line 14 col 5
// CHECK-NEXT:    bc 2249: line 56 col 6
// CHECK-NEXT:    bc 2254: line 14 col 5
// CHECK-NEXT:    bc 2258: line 56 col 15
// CHECK-NEXT:    bc 2263: line 14 col 5
// CHECK-NEXT:    bc 2267: line 56 col 24
// CHECK-NEXT:    bc 2273: line 14 col 5
// CHECK-NEXT:    bc 2277: line 56 col 33
// CHECK-NEXT:    bc 2283: line 14 col 5
// CHECK-NEXT:    bc 2287: line 56 col 42
// CHECK-NEXT:    bc 2292: line 14 col 5
// CHECK-NEXT:    bc 2298: line 12 col 7
// CHECK-NEXT:    bc 2305: line 58 col 12
// CHECK-NEXT:    bc 2310: line 58 col 7
// CHECK-NEXT:    bc 2314: line 60 col 11
// CHECK-NEXT:    bc 2320: line 60 col 7
// CHECK-NEXT:    bc 2324: line 61 col 11
// CHECK-NEXT:    bc 2330: line 61 col 7
// CHECK-NEXT:    bc 2334: line 62 col 11
// CHECK-NEXT:    bc 2340: line 62 col 7
// CHECK-NEXT:  0x07a2  function idx 2, starts at line 66 col 1
// CHECK-NEXT:    bc 6: line 68 col 8
// CHECK-NEXT:    bc 12: line 68 col 20
// CHECK-NEXT:    bc 18: line 68 col 32
// CHECK-NEXT:    bc 24: line 68 col 44
// CHECK-NEXT:    bc 30: line 68 col 56
// CHECK-NEXT:    bc 36: line 69 col 8
// CHECK-NEXT:    bc 42: line 69 col 20
// CHECK-NEXT:    bc 48: line 69 col 32
// CHECK-NEXT:    bc 54: line 69 col 44
// CHECK-NEXT:    bc 60: line 69 col 56
// CHECK-NEXT:    bc 66: line 70 col 9
// CHECK-NEXT:    bc 72: line 70 col 22
// CHECK-NEXT:    bc 78: line 70 col 35
// CHECK-NEXT:    bc 84: line 70 col 48
// CHECK-NEXT:    bc 90: line 70 col 61
// CHECK-NEXT:    bc 96: line 71 col 9
// CHECK-NEXT:    bc 102: line 71 col 22
// CHECK-NEXT:    bc 108: line 71 col 35
// CHECK-NEXT:    bc 114: line 71 col 48
// CHECK-NEXT:    bc 120: line 71 col 61
// CHECK-NEXT:    bc 126: line 72 col 9
// CHECK-NEXT:    bc 132: line 72 col 22
// CHECK-NEXT:    bc 138: line 72 col 35
// CHECK-NEXT:    bc 144: line 72 col 48
// CHECK-NEXT:    bc 150: line 72 col 61
// CHECK-NEXT:    bc 156: line 73 col 9
// CHECK-NEXT:    bc 162: line 73 col 22
// CHECK-NEXT:    bc 168: line 73 col 35
// CHECK-NEXT:    bc 174: line 73 col 48
// CHECK-NEXT:    bc 180: line 73 col 61
// CHECK-NEXT:    bc 186: line 74 col 9
// CHECK-NEXT:    bc 192: line 74 col 22
// CHECK-NEXT:    bc 198: line 74 col 35
// CHECK-NEXT:    bc 204: line 74 col 48
// CHECK-NEXT:    bc 210: line 74 col 61
// CHECK-NEXT:    bc 216: line 75 col 9
// CHECK-NEXT:    bc 222: line 75 col 22
// CHECK-NEXT:    bc 228: line 75 col 35
// CHECK-NEXT:    bc 234: line 75 col 48
// CHECK-NEXT:    bc 240: line 75 col 61
// CHECK-NEXT:    bc 246: line 76 col 9
// CHECK-NEXT:    bc 252: line 76 col 22
// CHECK-NEXT:    bc 258: line 76 col 35
// CHECK-NEXT:    bc 264: line 76 col 48
// CHECK-NEXT:    bc 270: line 76 col 61
// CHECK-NEXT:    bc 276: line 77 col 9
// CHECK-NEXT:    bc 282: line 77 col 22
// CHECK-NEXT:    bc 288: line 77 col 35
// CHECK-NEXT:    bc 294: line 77 col 48
// CHECK-NEXT:    bc 300: line 77 col 61
// CHECK-NEXT:    bc 306: line 78 col 9
// CHECK-NEXT:    bc 312: line 78 col 22
// CHECK-NEXT:    bc 318: line 78 col 35
// CHECK-NEXT:    bc 324: line 78 col 48
// CHECK-NEXT:    bc 330: line 78 col 61
// CHECK-NEXT:    bc 336: line 79 col 9
// CHECK-NEXT:    bc 342: line 79 col 22
// CHECK-NEXT:    bc 348: line 79 col 35
// CHECK-NEXT:    bc 354: line 79 col 48
// CHECK-NEXT:    bc 360: line 79 col 61
// CHECK-NEXT:    bc 366: line 80 col 9
// CHECK-NEXT:    bc 372: line 80 col 22
// CHECK-NEXT:    bc 378: line 80 col 35
// CHECK-NEXT:    bc 384: line 80 col 48
// CHECK-NEXT:    bc 390: line 80 col 61
// CHECK-NEXT:    bc 396: line 81 col 9
// CHECK-NEXT:    bc 402: line 81 col 22
// CHECK-NEXT:    bc 408: line 81 col 35
// CHECK-NEXT:    bc 414: line 81 col 48
// CHECK-NEXT:    bc 420: line 81 col 61
// CHECK-NEXT:    bc 426: line 82 col 9
// CHECK-NEXT:    bc 432: line 82 col 22
// CHECK-NEXT:    bc 438: line 82 col 35
// CHECK-NEXT:    bc 444: line 82 col 48
// CHECK-NEXT:    bc 450: line 82 col 61
// CHECK-NEXT:    bc 456: line 83 col 9
// CHECK-NEXT:    bc 462: line 83 col 22
// CHECK-NEXT:    bc 468: line 83 col 35
// CHECK-NEXT:    bc 474: line 83 col 48
// CHECK-NEXT:    bc 480: line 83 col 61
// CHECK-NEXT:    bc 486: line 84 col 9
// CHECK-NEXT:    bc 492: line 84 col 22
// CHECK-NEXT:    bc 498: line 84 col 35
// CHECK-NEXT:    bc 504: line 84 col 48
// CHECK-NEXT:    bc 510: line 84 col 61
// CHECK-NEXT:    bc 516: line 85 col 9
// CHECK-NEXT:    bc 522: line 85 col 22
// CHECK-NEXT:    bc 528: line 85 col 35
// CHECK-NEXT:    bc 534: line 85 col 48
// CHECK-NEXT:    bc 540: line 85 col 61
// CHECK-NEXT:    bc 546: line 86 col 9
// CHECK-NEXT:    bc 552: line 86 col 22
// CHECK-NEXT:    bc 558: line 86 col 35
// CHECK-NEXT:    bc 564: line 86 col 48
// CHECK-NEXT:    bc 570: line 86 col 61
// CHECK-NEXT:    bc 576: line 87 col 9
// CHECK-NEXT:    bc 582: line 87 col 22
// CHECK-NEXT:    bc 588: line 87 col 35
// CHECK-NEXT:    bc 594: line 87 col 48
// CHECK-NEXT:    bc 600: line 87 col 61
// CHECK-NEXT:    bc 606: line 89 col 10
// CHECK-NEXT:    bc 612: line 89 col 24
// CHECK-NEXT:    bc 618: line 89 col 38
// CHECK-NEXT:    bc 624: line 89 col 52
// CHECK-NEXT:    bc 630: line 89 col 66
// CHECK-NEXT:    bc 636: line 90 col 10
// CHECK-NEXT:    bc 642: line 90 col 24
// CHECK-NEXT:    bc 648: line 90 col 38
// CHECK-NEXT:    bc 654: line 90 col 52
// CHECK-NEXT:    bc 660: line 90 col 66
// CHECK-NEXT:    bc 666: line 91 col 10
// CHECK-NEXT:    bc 672: line 91 col 24
// CHECK-NEXT:    bc 678: line 91 col 38
// CHECK-NEXT:    bc 684: line 91 col 52
// CHECK-NEXT:    bc 690: line 91 col 66
// CHECK-NEXT:    bc 696: line 92 col 10
// CHECK-NEXT:    bc 702: line 92 col 24
// CHECK-NEXT:    bc 708: line 92 col 38
// CHECK-NEXT:    bc 714: line 92 col 52
// CHECK-NEXT:    bc 720: line 92 col 66
// CHECK-NEXT:    bc 726: line 93 col 10
// CHECK-NEXT:    bc 732: line 93 col 24
// CHECK-NEXT:    bc 738: line 93 col 38
// CHECK-NEXT:    bc 744: line 93 col 52
// CHECK-NEXT:    bc 750: line 93 col 66
// CHECK-NEXT:    bc 756: line 94 col 10
// CHECK-NEXT:    bc 762: line 94 col 24
// CHECK-NEXT:    bc 768: line 94 col 38
// CHECK-NEXT:    bc 774: line 94 col 52
// CHECK-NEXT:    bc 780: line 94 col 66
// CHECK-NEXT:    bc 786: line 95 col 10
// CHECK-NEXT:    bc 792: line 95 col 24
// CHECK-NEXT:    bc 798: line 95 col 38
// CHECK-NEXT:    bc 804: line 95 col 52
// CHECK-NEXT:    bc 810: line 95 col 66
// CHECK-NEXT:    bc 816: line 96 col 10
// CHECK-NEXT:    bc 822: line 96 col 24
// CHECK-NEXT:    bc 828: line 96 col 38
// CHECK-NEXT:    bc 834: line 96 col 52
// CHECK-NEXT:    bc 840: line 96 col 66
// CHECK-NEXT:    bc 846: line 97 col 10
// CHECK-NEXT:    bc 852: line 97 col 24
// CHECK-NEXT:    bc 858: line 97 col 38
// CHECK-NEXT:    bc 864: line 97 col 52
// CHECK-NEXT:    bc 870: line 97 col 66
// CHECK-NEXT:    bc 876: line 98 col 10
// CHECK-NEXT:    bc 882: line 98 col 24
// CHECK-NEXT:    bc 888: line 98 col 38
// CHECK-NEXT:    bc 894: line 98 col 52
// CHECK-NEXT:    bc 900: line 98 col 66
// CHECK-NEXT:    bc 906: line 99 col 10
// CHECK-NEXT:    bc 912: line 99 col 24
// CHECK-NEXT:    bc 918: line 99 col 38
// CHECK-NEXT:    bc 924: line 99 col 52
// CHECK-NEXT:    bc 930: line 99 col 66
// CHECK-NEXT:    bc 936: line 100 col 10
// CHECK-NEXT:    bc 942: line 100 col 24
// CHECK-NEXT:    bc 948: line 100 col 38
// CHECK-NEXT:    bc 954: line 100 col 52
// CHECK-NEXT:    bc 960: line 100 col 66
// CHECK-NEXT:    bc 966: line 101 col 10
// CHECK-NEXT:    bc 972: line 101 col 24
// CHECK-NEXT:    bc 978: line 101 col 38
// CHECK-NEXT:    bc 984: line 101 col 52
// CHECK-NEXT:    bc 990: line 101 col 66
// CHECK-NEXT:    bc 996: line 102 col 10
// CHECK-NEXT:    bc 1002: line 102 col 24
// CHECK-NEXT:    bc 1008: line 102 col 38
// CHECK-NEXT:    bc 1014: line 102 col 52
// CHECK-NEXT:    bc 1020: line 102 col 66
// CHECK-NEXT:    bc 1026: line 103 col 10
// CHECK-NEXT:    bc 1032: line 103 col 24
// CHECK-NEXT:    bc 1038: line 103 col 38
// CHECK-NEXT:    bc 1044: line 103 col 52
// CHECK-NEXT:    bc 1050: line 103 col 66
// CHECK-NEXT:    bc 1056: line 104 col 10
// CHECK-NEXT:    bc 1062: line 104 col 24
// CHECK-NEXT:    bc 1068: line 104 col 38
// CHECK-NEXT:    bc 1074: line 104 col 52
// CHECK-NEXT:    bc 1080: line 104 col 66
// CHECK-NEXT:    bc 1086: line 105 col 10
// CHECK-NEXT:    bc 1092: line 105 col 24
// CHECK-NEXT:    bc 1098: line 105 col 38
// CHECK-NEXT:    bc 1104: line 105 col 52
// CHECK-NEXT:    bc 1110: line 105 col 66
// CHECK-NEXT:    bc 1116: line 106 col 10
// CHECK-NEXT:    bc 1122: line 106 col 24
// CHECK-NEXT:    bc 1128: line 106 col 38
// CHECK-NEXT:    bc 1134: line 106 col 52
// CHECK-NEXT:    bc 1140: line 106 col 66
// CHECK-NEXT:    bc 1146: line 107 col 10
// CHECK-NEXT:    bc 1152: line 107 col 24
// CHECK-NEXT:    bc 1158: line 107 col 38
// CHECK-NEXT:    bc 1164: line 107 col 52
// CHECK-NEXT:    bc 1170: line 107 col 66
// CHECK-NEXT:    bc 1176: line 108 col 10
// CHECK-NEXT:    bc 1182: line 108 col 24
// CHECK-NEXT:    bc 1188: line 108 col 38
// CHECK-NEXT:    bc 1194: line 108 col 52
// CHECK-NEXT:    bc 1200: line 108 col 66
// CHECK-NEXT:    bc 1206: line 110 col 10
// CHECK-NEXT:    bc 1212: line 110 col 24
// CHECK-NEXT:    bc 1218: line 110 col 38
// CHECK-NEXT:    bc 1224: line 110 col 52
// CHECK-NEXT:    bc 1230: line 110 col 66
// CHECK-NEXT:    bc 1236: line 111 col 10
// CHECK-NEXT:    bc 1242: line 111 col 24
// CHECK-NEXT:    bc 1248: line 111 col 38
// CHECK-NEXT:    bc 1254: line 111 col 52
// CHECK-NEXT:    bc 1260: line 111 col 66
// CHECK-NEXT:    bc 1266: line 112 col 10
// CHECK-NEXT:    bc 1272: line 112 col 24
// CHECK-NEXT:    bc 1278: line 112 col 38
// CHECK-NEXT:    bc 1284: line 112 col 52
// CHECK-NEXT:    bc 1290: line 112 col 66
// CHECK-NEXT:    bc 1296: line 113 col 10
// CHECK-NEXT:    bc 1302: line 113 col 24
// CHECK-NEXT:    bc 1308: line 113 col 38
// CHECK-NEXT:    bc 1314: line 113 col 52
// CHECK-NEXT:    bc 1320: line 113 col 66
// CHECK-NEXT:    bc 1326: line 114 col 10
// CHECK-NEXT:    bc 1332: line 114 col 24
// CHECK-NEXT:    bc 1338: line 114 col 38
// CHECK-NEXT:    bc 1344: line 114 col 52
// CHECK-NEXT:    bc 1350: line 114 col 66
// CHECK-NEXT:    bc 1356: line 115 col 10
// CHECK-NEXT:    bc 1362: line 115 col 24
// CHECK-NEXT:    bc 1368: line 115 col 38
// CHECK-NEXT:    bc 1374: line 115 col 52
// CHECK-NEXT:    bc 1380: line 115 col 66
// CHECK-NEXT:    bc 1386: line 116 col 10
// CHECK-NEXT:    bc 1392: line 116 col 24
// CHECK-NEXT:    bc 1398: line 116 col 38
// CHECK-NEXT:    bc 1404: line 116 col 52
// CHECK-NEXT:    bc 1410: line 116 col 66
// CHECK-NEXT:    bc 1416: line 117 col 10
// CHECK-NEXT:    bc 1422: line 117 col 24
// CHECK-NEXT:    bc 1428: line 117 col 38
// CHECK-NEXT:    bc 1434: line 117 col 52
// CHECK-NEXT:    bc 1440: line 117 col 66
// CHECK-NEXT:    bc 1446: line 118 col 10
// CHECK-NEXT:    bc 1452: line 118 col 24
// CHECK-NEXT:    bc 1458: line 118 col 38
// CHECK-NEXT:    bc 1464: line 118 col 52
// CHECK-NEXT:    bc 1470: line 118 col 66
// CHECK-NEXT:    bc 1476: line 119 col 10
// CHECK-NEXT:    bc 1482: line 119 col 24
// CHECK-NEXT:    bc 1488: line 119 col 38
// CHECK-NEXT:    bc 1494: line 119 col 52
// CHECK-NEXT:    bc 1500: line 119 col 66
// CHECK-NEXT:    bc 1506: line 120 col 10
// CHECK-NEXT:    bc 1512: line 120 col 24
// CHECK-NEXT:    bc 1518: line 120 col 38
// CHECK-NEXT:    bc 1524: line 120 col 52
// CHECK-NEXT:    bc 1530: line 120 col 66
// CHECK-NEXT:    bc 1539: line 122 col 11
// CHECK-NEXT:    bc 1545: line 124 col 10
// CHECK-NEXT:    bc 1551: line 125 col 10
// CHECK-NEXT:    bc 1557: line 126 col 10
// CHECK-NEXT:  0x0ab1  end of debug source table
