/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -enable-asserts -target=HBC -dump-bytecode %s -O | %FileCheckOrRegen %s --match-full-lines

(function () {
  class ManyPrivateProperties {
    // First 100
    #f0 = 10; #f1 = 10; #f2 = 10; #f3 = 10; #f4 = 10;
    #f5 = 10; #f6 = 10; #f7 = 10; #f8 = 10; #f9 = 10;
    #f10 = 10; #f11 = 10; #f12 = 10; #f13 = 10; #f14 = 10;
    #f15 = 10; #f16 = 10; #f17 = 10; #f18 = 10; #f19 = 10;
    #f20 = 10; #f21 = 10; #f22 = 10; #f23 = 10; #f24 = 10;
    #f25 = 10; #f26 = 10; #f27 = 10; #f28 = 10; #f29 = 10;
    #f30 = 10; #f31 = 10; #f32 = 10; #f33 = 10; #f34 = 10;
    #f35 = 10; #f36 = 10; #f37 = 10; #f38 = 10; #f39 = 10;
    #f40 = 10; #f41 = 10; #f42 = 10; #f43 = 10; #f44 = 10;
    #f45 = 10; #f46 = 10; #f47 = 10; #f48 = 10; #f49 = 10;
    #f50 = 10; #f51 = 10; #f52 = 10; #f53 = 10; #f54 = 10;
    #f55 = 10; #f56 = 10; #f57 = 10; #f58 = 10; #f59 = 10;
    #f60 = 10; #f61 = 10; #f62 = 10; #f63 = 10; #f64 = 10;
    #f65 = 10; #f66 = 10; #f67 = 10; #f68 = 10; #f69 = 10;
    #f70 = 10; #f71 = 10; #f72 = 10; #f73 = 10; #f74 = 10;
    #f75 = 10; #f76 = 10; #f77 = 10; #f78 = 10; #f79 = 10;
    #f80 = 10; #f81 = 10; #f82 = 10; #f83 = 10; #f84 = 10;
    #f85 = 10; #f86 = 10; #f87 = 10; #f88 = 10; #f89 = 10;
    #f90 = 10; #f91 = 10; #f92 = 10; #f93 = 10; #f94 = 10;
    #f95 = 10; #f96 = 10; #f97 = 10; #f98 = 10; #f99 = 10;
    // Second 100
    #f100 = 10; #f101 = 10; #f102 = 10; #f103 = 10; #f104 = 10;
    #f105 = 10; #f106 = 10; #f107 = 10; #f108 = 10; #f109 = 10;
    #f110 = 10; #f111 = 10; #f112 = 10; #f113 = 10; #f114 = 10;
    #f115 = 10; #f116 = 10; #f117 = 10; #f118 = 10; #f119 = 10;
    #f120 = 10; #f121 = 10; #f122 = 10; #f123 = 10; #f124 = 10;
    #f125 = 10; #f126 = 10; #f127 = 10; #f128 = 10; #f129 = 10;
    #f130 = 10; #f131 = 10; #f132 = 10; #f133 = 10; #f134 = 10;
    #f135 = 10; #f136 = 10; #f137 = 10; #f138 = 10; #f139 = 10;
    #f140 = 10; #f141 = 10; #f142 = 10; #f143 = 10; #f144 = 10;
    #f145 = 10; #f146 = 10; #f147 = 10; #f148 = 10; #f149 = 10;
    #f150 = 10; #f151 = 10; #f152 = 10; #f153 = 10; #f154 = 10;
    #f155 = 10; #f156 = 10; #f157 = 10; #f158 = 10; #f159 = 10;
    #f160 = 10; #f161 = 10; #f162 = 10; #f163 = 10; #f164 = 10;
    #f165 = 10; #f166 = 10; #f167 = 10; #f168 = 10; #f169 = 10;
    #f170 = 10; #f171 = 10; #f172 = 10; #f173 = 10; #f174 = 10;
    #f175 = 10; #f176 = 10; #f177 = 10; #f178 = 10; #f179 = 10;
    #f180 = 10; #f181 = 10; #f182 = 10; #f183 = 10; #f184 = 10;
    #f185 = 10; #f186 = 10; #f187 = 10; #f188 = 10; #f189 = 10;
    #f190 = 10; #f191 = 10; #f192 = 10; #f193 = 10; #f194 = 10;
    #f195 = 10; #f196 = 10; #f197 = 10; #f198 = 10; #f199 = 10;
    // Last 55.
    #f200 = 10; #f201 = 10; #f202 = 10; #f203 = 10; #f204 = 10;
    #f205 = 10; #f206 = 10; #f207 = 10; #f208 = 10; #f209 = 10;
    #f210 = 10; #f211 = 10; #f212 = 10; #f213 = 10; #f214 = 10;
    #f215 = 10; #f216 = 10; #f217 = 10; #f218 = 10; #f219 = 10;
    #f220 = 10; #f221 = 10; #f222 = 10; #f223 = 10; #f224 = 10;
    #f225 = 10; #f226 = 10; #f227 = 10; #f228 = 10; #f229 = 10;
    #f230 = 10; #f231 = 10; #f232 = 10; #f233 = 10; #f234 = 10;
    #f235 = 10; #f236 = 10; #f237 = 10; #f238 = 10; #f239 = 10;
    #f240 = 10; #f241 = 10; #f242 = 10; #f243 = 10; #f244 = 10;
    #f245 = 10; #f246 = 10; #f247 = 10; #f248 = 10; #f249 = 10;
    #f250 = 10; #f251 = 10; #f252 = 10; #f253 = 10; #f254 = 10;
    // And a few more.
    #f255 = 10; #f256 = 10; #f257 = 10;

    // Now a function that gets them all.
    sum() {
      'noinline'
      var res =
          /* First 100*/
          this.#f0 + this.#f1 + this.#f2 + this.#f3 + this.#f4 +
          this.#f5 + this.#f6 + this.#f7 + this.#f8 + this.#f9 +
          this.#f10 + this.#f11 + this.#f12 + this.#f13 + this.#f14 +
          this.#f15 + this.#f16 + this.#f17 + this.#f18 + this.#f19 +
          this.#f20 + this.#f21 + this.#f22 + this.#f23 + this.#f24 +
          this.#f25 + this.#f26 + this.#f27 + this.#f28 + this.#f29 +
          this.#f30 + this.#f31 + this.#f32 + this.#f33 + this.#f34 +
          this.#f35 + this.#f36 + this.#f37 + this.#f38 + this.#f39 +
          this.#f40 + this.#f41 + this.#f42 + this.#f43 + this.#f44 +
          this.#f45 + this.#f46 + this.#f47 + this.#f48 + this.#f49 +
          this.#f50 + this.#f51 + this.#f52 + this.#f53 + this.#f54 +
          this.#f55 + this.#f56 + this.#f57 + this.#f58 + this.#f59 +
          this.#f60 + this.#f61 + this.#f62 + this.#f63 + this.#f64 +
          this.#f65 + this.#f66 + this.#f67 + this.#f68 + this.#f69 +
          this.#f70 + this.#f71 + this.#f72 + this.#f73 + this.#f74 +
          this.#f75 + this.#f76 + this.#f77 + this.#f78 + this.#f79 +
          this.#f80 + this.#f81 + this.#f82 + this.#f83 + this.#f84 +
          this.#f85 + this.#f86 + this.#f87 + this.#f88 + this.#f89 +
          this.#f90 + this.#f91 + this.#f92 + this.#f93 + this.#f94 +
          this.#f95 + this.#f96 + this.#f97 + this.#f98 + this.#f99 +
          // Second 100
          this.#f100 + this.#f101 + this.#f102 + this.#f103 + this.#f104 +
          this.#f105 + this.#f106 + this.#f107 + this.#f108 + this.#f109 +
          this.#f110 + this.#f111 + this.#f112 + this.#f113 + this.#f114 +
          this.#f115 + this.#f116 + this.#f117 + this.#f118 + this.#f119 +
          this.#f120 + this.#f121 + this.#f122 + this.#f123 + this.#f124 +
          this.#f125 + this.#f126 + this.#f127 + this.#f128 + this.#f129 +
          this.#f130 + this.#f131 + this.#f132 + this.#f133 + this.#f134 +
          this.#f135 + this.#f136 + this.#f137 + this.#f138 + this.#f139 +
          this.#f140 + this.#f141 + this.#f142 + this.#f143 + this.#f144 +
          this.#f145 + this.#f146 + this.#f147 + this.#f148 + this.#f149 +
          this.#f150 + this.#f151 + this.#f152 + this.#f153 + this.#f154 +
          this.#f155 + this.#f156 + this.#f157 + this.#f158 + this.#f159 +
          this.#f160 + this.#f161 + this.#f162 + this.#f163 + this.#f164 +
          this.#f165 + this.#f166 + this.#f167 + this.#f168 + this.#f169 +
          this.#f170 + this.#f171 + this.#f172 + this.#f173 + this.#f174 +
          this.#f175 + this.#f176 + this.#f177 + this.#f178 + this.#f179 +
          this.#f180 + this.#f181 + this.#f182 + this.#f183 + this.#f184 +
          this.#f185 + this.#f186 + this.#f187 + this.#f188 + this.#f189 +
          this.#f190 + this.#f191 + this.#f192 + this.#f193 + this.#f194 +
          this.#f195 + this.#f196 + this.#f197 + this.#f198 + this.#f199 +
          // Last 55
          this.#f200 + this.#f201 + this.#f202 + this.#f203 + this.#f204 +
          this.#f205 + this.#f206 + this.#f207 + this.#f208 + this.#f209 +
          this.#f210 + this.#f211 + this.#f212 + this.#f213 + this.#f214 +
          this.#f215 + this.#f216 + this.#f217 + this.#f218 + this.#f219 +
          this.#f220 + this.#f221 + this.#f222 + this.#f223 + this.#f224 +
          this.#f225 + this.#f226 + this.#f227 + this.#f228 + this.#f229 +
          this.#f230 + this.#f231 + this.#f232 + this.#f233 + this.#f234 +
          this.#f235 + this.#f236 + this.#f237 + this.#f238 + this.#f239 +
          this.#f240 + this.#f241 + this.#f242 + this.#f243 + this.#f244 +
          this.#f245 + this.#f246 + this.#f247 + this.#f248 + this.#f249 +
          this.#f250 + this.#f251 + this.#f252 + this.#f253 + this.#f254 +
          // Plus a few more.  These should use the overflow index.
          this.#f255 + this.#f256 + this.#f257;
      return res;
    }
  }
  var mpp = new ManyPrivateProperties();
  print(mpp.sum());
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 264
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 3
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:i0[ASCII, 0..2] #CF90938F: #f0
// CHECK-NEXT:i1[ASCII, 3..5] #CF90979E: #f1
// CHECK-NEXT:i2[ASCII, 3..6] #11A870F1: #f10
// CHECK-NEXT:i3[ASCII, 3..7] #B1A146F5: #f100
// CHECK-NEXT:i4[ASCII, 8..12] #B1A14AC6: #f101
// CHECK-NEXT:i5[ASCII, 13..17] #B1A14ED7: #f102
// CHECK-NEXT:i6[ASCII, 18..22] #B1A0B520: #f103
// CHECK-NEXT:i7[ASCII, 23..27] #B1A0B131: #f104
// CHECK-NEXT:i8[ASCII, 28..32] #B1A0BD02: #f105
// CHECK-NEXT:i9[ASCII, 33..37] #B1A0B913: #f106
// CHECK-NEXT:i10[ASCII, 38..42] #B1A0A56C: #f107
// CHECK-NEXT:i11[ASCII, 43..47] #B1A0A17D: #f108
// CHECK-NEXT:i12[ASCII, 48..52] #B1A0AD4E: #f109
// CHECK-NEXT:i13[ASCII, 53..56] #11A86C80: #f11
// CHECK-NEXT:i14[ASCII, 53..57] #B1964002: #f110
// CHECK-NEXT:i15[ASCII, 58..62] #B1965C73: #f111
// CHECK-NEXT:i16[ASCII, 63..67] #B1965860: #f112
// CHECK-NEXT:i17[ASCII, 68..72] #B1965451: #f113
// CHECK-NEXT:i18[ASCII, 73..77] #B1965046: #f114
// CHECK-NEXT:i19[ASCII, 78..82] #B1962DB7: #f115
// CHECK-NEXT:i20[ASCII, 83..87] #B19629A4: #f116
// CHECK-NEXT:i21[ASCII, 88..92] #B1962595: #f117
// CHECK-NEXT:i22[ASCII, 93..97] #B196218A: #f118
// CHECK-NEXT:i23[ASCII, 98..102] #B1963DFB: #f119
// CHECK-NEXT:i24[ASCII, 103..106] #11A8688F: #f12
// CHECK-NEXT:i25[ASCII, 103..107] #B186492D: #f120
// CHECK-NEXT:i26[ASCII, 108..112] #B1864563: #f121
// CHECK-NEXT:i27[ASCII, 113..117] #B1864172: #f122
// CHECK-NEXT:i28[ASCII, 118..122] #B1865D01: #f123
// CHECK-NEXT:i29[ASCII, 123..127] #B1865910: #f124
// CHECK-NEXT:i30[ASCII, 128..132] #B1865527: #f125
// CHECK-NEXT:i31[ASCII, 133..137] #B1865136: #f126
// CHECK-NEXT:i32[ASCII, 138..142] #B186AEC5: #f127
// CHECK-NEXT:i33[ASCII, 143..147] #B186AAD4: #f128
// CHECK-NEXT:i34[ASCII, 148..152] #B186A6EB: #f129
// CHECK-NEXT:i35[ASCII, 153..156] #11A864BE: #f13
// CHECK-NEXT:i36[ASCII, 153..157] #B1F0EC9D: #f130
// CHECK-NEXT:i37[ASCII, 158..162] #B1F0D06C: #f131
// CHECK-NEXT:i38[ASCII, 163..167] #B1F0D463: #f132
// CHECK-NEXT:i39[ASCII, 168..172] #B1F0D852: #f133
// CHECK-NEXT:i40[ASCII, 173..177] #B1F0DC41: #f134
// CHECK-NEXT:i41[ASCII, 178..182] #B1F0C030: #f135
// CHECK-NEXT:i42[ASCII, 183..187] #B1F0C427: #f136
// CHECK-NEXT:i43[ASCII, 188..192] #B1F0C816: #f137
// CHECK-NEXT:i44[ASCII, 193..197] #B1F0CC05: #f138
// CHECK-NEXT:i45[ASCII, 198..202] #B1F0B1F4: #f139
// CHECK-NEXT:i46[ASCII, 203..206] #11A860AD: #f14
// CHECK-NEXT:i47[ASCII, 203..207] #B1E77B8E: #f140
// CHECK-NEXT:i48[ASCII, 208..212] #B1E777BD: #f141
// CHECK-NEXT:i49[ASCII, 213..217] #B1E773AC: #f142
// CHECK-NEXT:i50[ASCII, 218..222] #B1E74F63: #f143
// CHECK-NEXT:i51[ASCII, 223..227] #B1E74B72: #f144
// CHECK-NEXT:i52[ASCII, 228..232] #B1E74741: #f145
// CHECK-NEXT:i53[ASCII, 233..237] #B1E74350: #f146
// CHECK-NEXT:i54[ASCII, 238..242] #B1E75F27: #f147
// CHECK-NEXT:i55[ASCII, 243..247] #B1E75B36: #f148
// CHECK-NEXT:i56[ASCII, 248..252] #B1E75705: #f149
// CHECK-NEXT:i57[ASCII, 253..256] #11A85C5C: #f15
// CHECK-NEXT:i58[ASCII, 253..257] #B1D6E6BE: #f150
// CHECK-NEXT:i59[ASCII, 258..262] #B1D6FACF: #f151
// CHECK-NEXT:i60[ASCII, 263..267] #B1D6FEDC: #f152
// CHECK-NEXT:i61[ASCII, 268..272] #B1D6F2ED: #f153
// CHECK-NEXT:i62[ASCII, 273..277] #B1D6F6E2: #f154
// CHECK-NEXT:i63[ASCII, 278..282] #B1D6CA13: #f155
// CHECK-NEXT:i64[ASCII, 283..287] #B1D6CE00: #f156
// CHECK-NEXT:i65[ASCII, 288..292] #B1D6C231: #f157
// CHECK-NEXT:i66[ASCII, 293..297] #B1D6C626: #f158
// CHECK-NEXT:i67[ASCII, 298..302] #B1D6DA57: #f159
// CHECK-NEXT:i68[ASCII, 303..306] #11A8584B: #f16
// CHECK-NEXT:i69[ASCII, 303..307] #B1C66D6A: #f160
// CHECK-NEXT:i70[ASCII, 308..312] #B1C6615D: #f161
// CHECK-NEXT:i71[ASCII, 313..317] #B1C6654C: #f162
// CHECK-NEXT:i72[ASCII, 318..322] #B1C6793F: #f163
// CHECK-NEXT:i73[ASCII, 323..327] #B1C67D2E: #f164
// CHECK-NEXT:i74[ASCII, 328..332] #B1C671E2: #f165
// CHECK-NEXT:i75[ASCII, 333..337] #B1C675F3: #f166
// CHECK-NEXT:i76[ASCII, 338..342] #B1C64900: #f167
// CHECK-NEXT:i77[ASCII, 343..347] #B1C64D11: #f168
// CHECK-NEXT:i78[ASCII, 348..352] #B1C64126: #f169
// CHECK-NEXT:i79[ASCII, 353..356] #11A8547A: #f17
// CHECK-NEXT:i80[ASCII, 353..357] #B0311758: #f170
// CHECK-NEXT:i81[ASCII, 358..362] #B030ECA9: #f171
// CHECK-NEXT:i82[ASCII, 363..367] #B030E8BE: #f172
// CHECK-NEXT:i83[ASCII, 368..372] #B030E48F: #f173
// CHECK-NEXT:i84[ASCII, 373..377] #B030E09C: #f174
// CHECK-NEXT:i85[ASCII, 378..382] #B030FCED: #f175
// CHECK-NEXT:i86[ASCII, 383..387] #B030F8E2: #f176
// CHECK-NEXT:i87[ASCII, 388..392] #B030F4D3: #f177
// CHECK-NEXT:i88[ASCII, 393..397] #B030F0C0: #f178
// CHECK-NEXT:i89[ASCII, 398..402] #B030CC31: #f179
// CHECK-NEXT:i90[ASCII, 403..406] #11A85069: #f18
// CHECK-NEXT:i91[ASCII, 403..407] #B0211E4B: #f180
// CHECK-NEXT:i92[ASCII, 408..412] #B0211278: #f181
// CHECK-NEXT:i93[ASCII, 413..417] #B0211669: #f182
// CHECK-NEXT:i94[ASCII, 418..422] #B0216B9E: #f183
// CHECK-NEXT:i95[ASCII, 423..427] #B0216F8F: #f184
// CHECK-NEXT:i96[ASCII, 428..432] #B02163BC: #f185
// CHECK-NEXT:i97[ASCII, 433..437] #B02167AD: #f186
// CHECK-NEXT:i98[ASCII, 438..442] #B0217BE2: #f187
// CHECK-NEXT:i99[ASCII, 443..447] #B0217FF3: #f188
// CHECK-NEXT:i100[ASCII, 448..452] #B02173C0: #f189
// CHECK-NEXT:i101[ASCII, 453..456] #11A84C18: #f19
// CHECK-NEXT:i102[ASCII, 453..457] #B01209F9: #f190
// CHECK-NEXT:i103[ASCII, 458..462] #B0121588: #f191
// CHECK-NEXT:i104[ASCII, 463..467] #B012119B: #f192
// CHECK-NEXT:i105[ASCII, 468..472] #B0121DAA: #f193
// CHECK-NEXT:i106[ASCII, 473..477] #B01219BD: #f194
// CHECK-NEXT:i107[ASCII, 478..482] #B012E64C: #f195
// CHECK-NEXT:i108[ASCII, 483..487] #B012E25F: #f196
// CHECK-NEXT:i109[ASCII, 488..492] #B012EE6E: #f197
// CHECK-NEXT:i110[ASCII, 493..497] #B012EA61: #f198
// CHECK-NEXT:i111[ASCII, 498..502] #B012F610: #f199
// CHECK-NEXT:i112[ASCII, 503..505] #CF90AB6D: #f2
// CHECK-NEXT:i113[ASCII, 503..506] #1277E3E3: #f20
// CHECK-NEXT:i114[ASCII, 503..507] #F1C010D3: #f200
// CHECK-NEXT:i115[ASCII, 508..512] #F1C014C4: #f201
// CHECK-NEXT:i116[ASCII, 513..517] #F1C018F5: #f202
// CHECK-NEXT:i117[ASCII, 518..522] #F1C01CE6: #f203
// CHECK-NEXT:i118[ASCII, 523..527] #F1C06117: #f204
// CHECK-NEXT:i119[ASCII, 528..532] #F1C06508: #f205
// CHECK-NEXT:i120[ASCII, 533..537] #F1C06939: #f206
// CHECK-NEXT:i121[ASCII, 538..542] #F1C06D2A: #f207
// CHECK-NEXT:i122[ASCII, 543..547] #F1C0715B: #f208
// CHECK-NEXT:i123[ASCII, 548..552] #F1C0754C: #f209
// CHECK-NEXT:i124[ASCII, 553..556] #1277DF10: #f21
// CHECK-NEXT:i125[ASCII, 553..557] #F2330C3D: #f210
// CHECK-NEXT:i126[ASCII, 558..562] #F23330CC: #f211
// CHECK-NEXT:i127[ASCII, 563..567] #F23334DF: #f212
// CHECK-NEXT:i128[ASCII, 568..572] #F23338EE: #f213
// CHECK-NEXT:i129[ASCII, 573..577] #F2333CF9: #f214
// CHECK-NEXT:i130[ASCII, 578..582] #F2332088: #f215
// CHECK-NEXT:i131[ASCII, 583..587] #F233249B: #f216
// CHECK-NEXT:i132[ASCII, 588..592] #F23328AA: #f217
// CHECK-NEXT:i133[ASCII, 593..597] #F2332CB5: #f218
// CHECK-NEXT:i134[ASCII, 598..602] #F232D744: #f219
// CHECK-NEXT:i135[ASCII, 603..606] #1277DB01: #f22
// CHECK-NEXT:i136[ASCII, 603..607] #F2230D4D: #f220
// CHECK-NEXT:i137[ASCII, 608..612] #F22331BE: #f221
// CHECK-NEXT:i138[ASCII, 613..617] #F22335AF: #f222
// CHECK-NEXT:i139[ASCII, 618..622] #F2233998: #f223
// CHECK-NEXT:i140[ASCII, 623..627] #F2233D89: #f224
// CHECK-NEXT:i141[ASCII, 628..632] #F22321FA: #f225
// CHECK-NEXT:i142[ASCII, 633..637] #F22325EB: #f226
// CHECK-NEXT:i143[ASCII, 638..642] #F22329D4: #f227
// CHECK-NEXT:i144[ASCII, 643..647] #F2232DC5: #f228
// CHECK-NEXT:i145[ASCII, 648..652] #F2235036: #f229
// CHECK-NEXT:i146[ASCII, 653..656] #1277D70E: #f23
// CHECK-NEXT:i147[ASCII, 653..657] #F2139C02: #f230
// CHECK-NEXT:i148[ASCII, 658..662] #F2138073: #f231
// CHECK-NEXT:i149[ASCII, 663..667] #F213841D: #f232
// CHECK-NEXT:i150[ASCII, 668..672] #F213882C: #f233
// CHECK-NEXT:i151[ASCII, 673..677] #F2138C3F: #f234
// CHECK-NEXT:i152[ASCII, 678..682] #F213B0CE: #f235
// CHECK-NEXT:i153[ASCII, 683..687] #F213B4D9: #f236
// CHECK-NEXT:i154[ASCII, 688..692] #F213B8E8: #f237
// CHECK-NEXT:i155[ASCII, 693..697] #F213BCFB: #f238
// CHECK-NEXT:i156[ASCII, 698..702] #F213A08A: #f239
// CHECK-NEXT:i157[ASCII, 703..706] #1277D31F: #f24
// CHECK-NEXT:i158[ASCII, 703..707] #F2021B72: #f240
// CHECK-NEXT:i159[ASCII, 708..712] #F202071D: #f241
// CHECK-NEXT:i160[ASCII, 713..717] #F202030C: #f242
// CHECK-NEXT:i161[ASCII, 718..722] #F2020F3F: #f243
// CHECK-NEXT:i162[ASCII, 723..727] #F2020B2E: #f244
// CHECK-NEXT:i163[ASCII, 728..732] #F20237D9: #f245
// CHECK-NEXT:i164[ASCII, 733..737] #F20233C8: #f246
// CHECK-NEXT:i165[ASCII, 738..742] #F2023FFB: #f247
// CHECK-NEXT:i166[ASCII, 743..747] #F2023BEA: #f248
// CHECK-NEXT:i167[ASCII, 748..752] #F2022795: #f249
// CHECK-NEXT:i168[ASCII, 753..756] #1277CF6C: #f25
// CHECK-NEXT:i169[ASCII, 753..757] #F270E762: #f250
// CHECK-NEXT:i170[ASCII, 758..762] #F2709A93: #f251
// CHECK-NEXT:i171[ASCII, 763..767] #F2709E80: #f252
// CHECK-NEXT:i172[ASCII, 768..772] #F27092B1: #f253
// CHECK-NEXT:i173[ASCII, 773..776] #1277CB7D: #f26
// CHECK-NEXT:i174[ASCII, 777..780] #1277C74A: #f27
// CHECK-NEXT:i175[ASCII, 781..784] #1277C35B: #f28
// CHECK-NEXT:i176[ASCII, 785..788] #1277BEA8: #f29
// CHECK-NEXT:i177[ASCII, 789..791] #CF90AF7C: #f3
// CHECK-NEXT:i178[ASCII, 789..792] #120662D2: #f30
// CHECK-NEXT:i179[ASCII, 793..796] #12065E23: #f31
// CHECK-NEXT:i180[ASCII, 797..800] #12065A30: #f32
// CHECK-NEXT:i181[ASCII, 801..804] #12065601: #f33
// CHECK-NEXT:i182[ASCII, 805..808] #1206520E: #f34
// CHECK-NEXT:i183[ASCII, 809..812] #12064E7F: #f35
// CHECK-NEXT:i184[ASCII, 813..816] #12064A6C: #f36
// CHECK-NEXT:i185[ASCII, 817..820] #1206465D: #f37
// CHECK-NEXT:i186[ASCII, 821..824] #1206424A: #f38
// CHECK-NEXT:i187[ASCII, 825..828] #1206BDBB: #f39
// CHECK-NEXT:i188[ASCII, 829..831] #CF90A34B: #f4
// CHECK-NEXT:i189[ASCII, 829..832] #1256F546: #f40
// CHECK-NEXT:i190[ASCII, 833..836] #1256E931: #f41
// CHECK-NEXT:i191[ASCII, 837..840] #1256ED20: #f42
// CHECK-NEXT:i192[ASCII, 841..844] #1256E113: #f43
// CHECK-NEXT:i193[ASCII, 845..848] #1256E502: #f44
// CHECK-NEXT:i194[ASCII, 849..852] #1256D90E: #f45
// CHECK-NEXT:i195[ASCII, 853..856] #1256DD1F: #f46
// CHECK-NEXT:i196[ASCII, 857..860] #1256D12C: #f47
// CHECK-NEXT:i197[ASCII, 861..864] #1256D53D: #f48
// CHECK-NEXT:i198[ASCII, 865..868] #1256C94A: #f49
// CHECK-NEXT:i199[ASCII, 869..871] #CF90A75A: #f5
// CHECK-NEXT:i200[ASCII, 869..872] #126674B4: #f50
// CHECK-NEXT:i201[ASCII, 873..876] #126668C5: #f51
// CHECK-NEXT:i202[ASCII, 877..880] #12666CD2: #f52
// CHECK-NEXT:i203[ASCII, 881..884] #126660E3: #f53
// CHECK-NEXT:i204[ASCII, 885..888] #126664F0: #f54
// CHECK-NEXT:i205[ASCII, 889..892] #12665801: #f55
// CHECK-NEXT:i206[ASCII, 893..896] #12665C0E: #f56
// CHECK-NEXT:i207[ASCII, 897..900] #1266503F: #f57
// CHECK-NEXT:i208[ASCII, 901..904] #1266542C: #f58
// CHECK-NEXT:i209[ASCII, 905..908] #1266485D: #f59
// CHECK-NEXT:i210[ASCII, 909..911] #CF90BB29: #f6
// CHECK-NEXT:i211[ASCII, 909..912] #1237E724: #f60
// CHECK-NEXT:i212[ASCII, 913..916] #1237DBD7: #f61
// CHECK-NEXT:i213[ASCII, 917..920] #1237DFC6: #f62
// CHECK-NEXT:i214[ASCII, 921..924] #1237D3F1: #f63
// CHECK-NEXT:i215[ASCII, 925..928] #1237D7E0: #f64
// CHECK-NEXT:i216[ASCII, 929..932] #1237CB93: #f65
// CHECK-NEXT:i217[ASCII, 933..936] #1237CF82: #f66
// CHECK-NEXT:i218[ASCII, 937..940] #1237C38D: #f67
// CHECK-NEXT:i219[ASCII, 941..944] #1237C79C: #f68
// CHECK-NEXT:i220[ASCII, 945..948] #1237BA6F: #f69
// CHECK-NEXT:i221[ASCII, 949..951] #CF90BF38: #f7
// CHECK-NEXT:i222[ASCII, 949..952] #12C46615: #f70
// CHECK-NEXT:i223[ASCII, 953..956] #12C45AE4: #f71
// CHECK-NEXT:i224[ASCII, 957..960] #12C45EF7: #f72
// CHECK-NEXT:i225[ASCII, 961..964] #12C452C6: #f73
// CHECK-NEXT:i226[ASCII, 965..968] #12C456D1: #f74
// CHECK-NEXT:i227[ASCII, 969..972] #12C44AA0: #f75
// CHECK-NEXT:i228[ASCII, 973..976] #12C44EB3: #f76
// CHECK-NEXT:i229[ASCII, 977..980] #12C44282: #f77
// CHECK-NEXT:i230[ASCII, 981..984] #12C4468D: #f78
// CHECK-NEXT:i231[ASCII, 985..988] #12C4B97C: #f79
// CHECK-NEXT:i232[ASCII, 989..991] #CF90B307: #f8
// CHECK-NEXT:i233[ASCII, 989..992] #1214F90B: #f80
// CHECK-NEXT:i234[ASCII, 993..996] #1214E574: #f81
// CHECK-NEXT:i235[ASCII, 997..1000] #1214E165: #f82
// CHECK-NEXT:i236[ASCII, 1001..1004] #1214ED56: #f83
// CHECK-NEXT:i237[ASCII, 1005..1008] #1214E947: #f84
// CHECK-NEXT:i238[ASCII, 1009..1012] #1214D5B0: #f85
// CHECK-NEXT:i239[ASCII, 1013..1016] #1214D1A1: #f86
// CHECK-NEXT:i240[ASCII, 1017..1020] #1214DD92: #f87
// CHECK-NEXT:i241[ASCII, 1021..1024] #1214D983: #f88
// CHECK-NEXT:i242[ASCII, 1025..1028] #1214C58D: #f89
// CHECK-NEXT:i243[ASCII, 1029..1031] #CF90B716: #f9
// CHECK-NEXT:i244[ASCII, 1029..1032] #1224787B: #f90
// CHECK-NEXT:i245[ASCII, 1033..1036] #1224640A: #f91
// CHECK-NEXT:i246[ASCII, 1037..1040] #12246015: #f92
// CHECK-NEXT:i247[ASCII, 1041..1044] #12246C24: #f93
// CHECK-NEXT:i248[ASCII, 1045..1048] #12246837: #f94
// CHECK-NEXT:i249[ASCII, 1049..1052] #122454C6: #f95
// CHECK-NEXT:i250[ASCII, 1053..1056] #122450D1: #f96
// CHECK-NEXT:i251[ASCII, 1057..1060] #12245CE0: #f97
// CHECK-NEXT:i252[ASCII, 1061..1064] #122458F3: #f98
// CHECK-NEXT:i253[ASCII, 1065..1068] #12244482: #f99
// CHECK-NEXT:i254[ASCII, 1089..1091] #BA21B760: sum
// CHECK-NEXT:s255[ASCII, 1069..1089]: ManyPrivateProperties
// CHECK-NEXT:s256[ASCII, 1092..1129]: Cannot initialize private field twice.
// CHECK-NEXT:s257[ASCII, 1130..1135]: global
// CHECK-NEXT:i258[ASCII, 1136..1140] #F270969E: #f254
// CHECK-NEXT:i259[ASCII, 1141..1145] #F2708AEF: #f255
// CHECK-NEXT:i260[ASCII, 1146..1150] #F2708EFC: #f256
// CHECK-NEXT:i261[ASCII, 1151..1155] #F27082CD: #f257
// CHECK-NEXT:i262[ASCII, 1156..1160] #A689F65B: print
// CHECK-NEXT:i263[ASCII, 1161..1169] #807C5F3D: prototype

// CHECK:Function<global>(1 params, 14 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r2, 258
// CHECK-NEXT:    CreatePrivateName r1, "#f0"
// CHECK-NEXT:    StoreToEnvironment r2, 0, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f1"
// CHECK-NEXT:    StoreToEnvironment r2, 1, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f2"
// CHECK-NEXT:    StoreToEnvironment r2, 2, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f3"
// CHECK-NEXT:    StoreToEnvironment r2, 3, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f4"
// CHECK-NEXT:    StoreToEnvironment r2, 4, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f5"
// CHECK-NEXT:    StoreToEnvironment r2, 5, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f6"
// CHECK-NEXT:    StoreToEnvironment r2, 6, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f7"
// CHECK-NEXT:    StoreToEnvironment r2, 7, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f8"
// CHECK-NEXT:    StoreToEnvironment r2, 8, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f9"
// CHECK-NEXT:    StoreToEnvironment r2, 9, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f10"
// CHECK-NEXT:    StoreToEnvironment r2, 10, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f11"
// CHECK-NEXT:    StoreToEnvironment r2, 11, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f12"
// CHECK-NEXT:    StoreToEnvironment r2, 12, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f13"
// CHECK-NEXT:    StoreToEnvironment r2, 13, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f14"
// CHECK-NEXT:    StoreToEnvironment r2, 14, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f15"
// CHECK-NEXT:    StoreToEnvironment r2, 15, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f16"
// CHECK-NEXT:    StoreToEnvironment r2, 16, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f17"
// CHECK-NEXT:    StoreToEnvironment r2, 17, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f18"
// CHECK-NEXT:    StoreToEnvironment r2, 18, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f19"
// CHECK-NEXT:    StoreToEnvironment r2, 19, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f20"
// CHECK-NEXT:    StoreToEnvironment r2, 20, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f21"
// CHECK-NEXT:    StoreToEnvironment r2, 21, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f22"
// CHECK-NEXT:    StoreToEnvironment r2, 22, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f23"
// CHECK-NEXT:    StoreToEnvironment r2, 23, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f24"
// CHECK-NEXT:    StoreToEnvironment r2, 24, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f25"
// CHECK-NEXT:    StoreToEnvironment r2, 25, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f26"
// CHECK-NEXT:    StoreToEnvironment r2, 26, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f27"
// CHECK-NEXT:    StoreToEnvironment r2, 27, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f28"
// CHECK-NEXT:    StoreToEnvironment r2, 28, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f29"
// CHECK-NEXT:    StoreToEnvironment r2, 29, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f30"
// CHECK-NEXT:    StoreToEnvironment r2, 30, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f31"
// CHECK-NEXT:    StoreToEnvironment r2, 31, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f32"
// CHECK-NEXT:    StoreToEnvironment r2, 32, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f33"
// CHECK-NEXT:    StoreToEnvironment r2, 33, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f34"
// CHECK-NEXT:    StoreToEnvironment r2, 34, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f35"
// CHECK-NEXT:    StoreToEnvironment r2, 35, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f36"
// CHECK-NEXT:    StoreToEnvironment r2, 36, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f37"
// CHECK-NEXT:    StoreToEnvironment r2, 37, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f38"
// CHECK-NEXT:    StoreToEnvironment r2, 38, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f39"
// CHECK-NEXT:    StoreToEnvironment r2, 39, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f40"
// CHECK-NEXT:    StoreToEnvironment r2, 40, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f41"
// CHECK-NEXT:    StoreToEnvironment r2, 41, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f42"
// CHECK-NEXT:    StoreToEnvironment r2, 42, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f43"
// CHECK-NEXT:    StoreToEnvironment r2, 43, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f44"
// CHECK-NEXT:    StoreToEnvironment r2, 44, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f45"
// CHECK-NEXT:    StoreToEnvironment r2, 45, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f46"
// CHECK-NEXT:    StoreToEnvironment r2, 46, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f47"
// CHECK-NEXT:    StoreToEnvironment r2, 47, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f48"
// CHECK-NEXT:    StoreToEnvironment r2, 48, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f49"
// CHECK-NEXT:    StoreToEnvironment r2, 49, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f50"
// CHECK-NEXT:    StoreToEnvironment r2, 50, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f51"
// CHECK-NEXT:    StoreToEnvironment r2, 51, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f52"
// CHECK-NEXT:    StoreToEnvironment r2, 52, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f53"
// CHECK-NEXT:    StoreToEnvironment r2, 53, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f54"
// CHECK-NEXT:    StoreToEnvironment r2, 54, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f55"
// CHECK-NEXT:    StoreToEnvironment r2, 55, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f56"
// CHECK-NEXT:    StoreToEnvironment r2, 56, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f57"
// CHECK-NEXT:    StoreToEnvironment r2, 57, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f58"
// CHECK-NEXT:    StoreToEnvironment r2, 58, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f59"
// CHECK-NEXT:    StoreToEnvironment r2, 59, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f60"
// CHECK-NEXT:    StoreToEnvironment r2, 60, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f61"
// CHECK-NEXT:    StoreToEnvironment r2, 61, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f62"
// CHECK-NEXT:    StoreToEnvironment r2, 62, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f63"
// CHECK-NEXT:    StoreToEnvironment r2, 63, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f64"
// CHECK-NEXT:    StoreToEnvironment r2, 64, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f65"
// CHECK-NEXT:    StoreToEnvironment r2, 65, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f66"
// CHECK-NEXT:    StoreToEnvironment r2, 66, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f67"
// CHECK-NEXT:    StoreToEnvironment r2, 67, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f68"
// CHECK-NEXT:    StoreToEnvironment r2, 68, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f69"
// CHECK-NEXT:    StoreToEnvironment r2, 69, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f70"
// CHECK-NEXT:    StoreToEnvironment r2, 70, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f71"
// CHECK-NEXT:    StoreToEnvironment r2, 71, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f72"
// CHECK-NEXT:    StoreToEnvironment r2, 72, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f73"
// CHECK-NEXT:    StoreToEnvironment r2, 73, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f74"
// CHECK-NEXT:    StoreToEnvironment r2, 74, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f75"
// CHECK-NEXT:    StoreToEnvironment r2, 75, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f76"
// CHECK-NEXT:    StoreToEnvironment r2, 76, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f77"
// CHECK-NEXT:    StoreToEnvironment r2, 77, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f78"
// CHECK-NEXT:    StoreToEnvironment r2, 78, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f79"
// CHECK-NEXT:    StoreToEnvironment r2, 79, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f80"
// CHECK-NEXT:    StoreToEnvironment r2, 80, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f81"
// CHECK-NEXT:    StoreToEnvironment r2, 81, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f82"
// CHECK-NEXT:    StoreToEnvironment r2, 82, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f83"
// CHECK-NEXT:    StoreToEnvironment r2, 83, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f84"
// CHECK-NEXT:    StoreToEnvironment r2, 84, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f85"
// CHECK-NEXT:    StoreToEnvironment r2, 85, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f86"
// CHECK-NEXT:    StoreToEnvironment r2, 86, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f87"
// CHECK-NEXT:    StoreToEnvironment r2, 87, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f88"
// CHECK-NEXT:    StoreToEnvironment r2, 88, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f89"
// CHECK-NEXT:    StoreToEnvironment r2, 89, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f90"
// CHECK-NEXT:    StoreToEnvironment r2, 90, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f91"
// CHECK-NEXT:    StoreToEnvironment r2, 91, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f92"
// CHECK-NEXT:    StoreToEnvironment r2, 92, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f93"
// CHECK-NEXT:    StoreToEnvironment r2, 93, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f94"
// CHECK-NEXT:    StoreToEnvironment r2, 94, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f95"
// CHECK-NEXT:    StoreToEnvironment r2, 95, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f96"
// CHECK-NEXT:    StoreToEnvironment r2, 96, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f97"
// CHECK-NEXT:    StoreToEnvironment r2, 97, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f98"
// CHECK-NEXT:    StoreToEnvironment r2, 98, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f99"
// CHECK-NEXT:    StoreToEnvironment r2, 99, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f100"
// CHECK-NEXT:    StoreToEnvironment r2, 100, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f101"
// CHECK-NEXT:    StoreToEnvironment r2, 101, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f102"
// CHECK-NEXT:    StoreToEnvironment r2, 102, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f103"
// CHECK-NEXT:    StoreToEnvironment r2, 103, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f104"
// CHECK-NEXT:    StoreToEnvironment r2, 104, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f105"
// CHECK-NEXT:    StoreToEnvironment r2, 105, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f106"
// CHECK-NEXT:    StoreToEnvironment r2, 106, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f107"
// CHECK-NEXT:    StoreToEnvironment r2, 107, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f108"
// CHECK-NEXT:    StoreToEnvironment r2, 108, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f109"
// CHECK-NEXT:    StoreToEnvironment r2, 109, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f110"
// CHECK-NEXT:    StoreToEnvironment r2, 110, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f111"
// CHECK-NEXT:    StoreToEnvironment r2, 111, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f112"
// CHECK-NEXT:    StoreToEnvironment r2, 112, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f113"
// CHECK-NEXT:    StoreToEnvironment r2, 113, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f114"
// CHECK-NEXT:    StoreToEnvironment r2, 114, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f115"
// CHECK-NEXT:    StoreToEnvironment r2, 115, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f116"
// CHECK-NEXT:    StoreToEnvironment r2, 116, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f117"
// CHECK-NEXT:    StoreToEnvironment r2, 117, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f118"
// CHECK-NEXT:    StoreToEnvironment r2, 118, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f119"
// CHECK-NEXT:    StoreToEnvironment r2, 119, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f120"
// CHECK-NEXT:    StoreToEnvironment r2, 120, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f121"
// CHECK-NEXT:    StoreToEnvironment r2, 121, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f122"
// CHECK-NEXT:    StoreToEnvironment r2, 122, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f123"
// CHECK-NEXT:    StoreToEnvironment r2, 123, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f124"
// CHECK-NEXT:    StoreToEnvironment r2, 124, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f125"
// CHECK-NEXT:    StoreToEnvironment r2, 125, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f126"
// CHECK-NEXT:    StoreToEnvironment r2, 126, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f127"
// CHECK-NEXT:    StoreToEnvironment r2, 127, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f128"
// CHECK-NEXT:    StoreToEnvironment r2, 128, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f129"
// CHECK-NEXT:    StoreToEnvironment r2, 129, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f130"
// CHECK-NEXT:    StoreToEnvironment r2, 130, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f131"
// CHECK-NEXT:    StoreToEnvironment r2, 131, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f132"
// CHECK-NEXT:    StoreToEnvironment r2, 132, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f133"
// CHECK-NEXT:    StoreToEnvironment r2, 133, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f134"
// CHECK-NEXT:    StoreToEnvironment r2, 134, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f135"
// CHECK-NEXT:    StoreToEnvironment r2, 135, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f136"
// CHECK-NEXT:    StoreToEnvironment r2, 136, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f137"
// CHECK-NEXT:    StoreToEnvironment r2, 137, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f138"
// CHECK-NEXT:    StoreToEnvironment r2, 138, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f139"
// CHECK-NEXT:    StoreToEnvironment r2, 139, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f140"
// CHECK-NEXT:    StoreToEnvironment r2, 140, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f141"
// CHECK-NEXT:    StoreToEnvironment r2, 141, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f142"
// CHECK-NEXT:    StoreToEnvironment r2, 142, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f143"
// CHECK-NEXT:    StoreToEnvironment r2, 143, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f144"
// CHECK-NEXT:    StoreToEnvironment r2, 144, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f145"
// CHECK-NEXT:    StoreToEnvironment r2, 145, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f146"
// CHECK-NEXT:    StoreToEnvironment r2, 146, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f147"
// CHECK-NEXT:    StoreToEnvironment r2, 147, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f148"
// CHECK-NEXT:    StoreToEnvironment r2, 148, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f149"
// CHECK-NEXT:    StoreToEnvironment r2, 149, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f150"
// CHECK-NEXT:    StoreToEnvironment r2, 150, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f151"
// CHECK-NEXT:    StoreToEnvironment r2, 151, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f152"
// CHECK-NEXT:    StoreToEnvironment r2, 152, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f153"
// CHECK-NEXT:    StoreToEnvironment r2, 153, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f154"
// CHECK-NEXT:    StoreToEnvironment r2, 154, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f155"
// CHECK-NEXT:    StoreToEnvironment r2, 155, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f156"
// CHECK-NEXT:    StoreToEnvironment r2, 156, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f157"
// CHECK-NEXT:    StoreToEnvironment r2, 157, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f158"
// CHECK-NEXT:    StoreToEnvironment r2, 158, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f159"
// CHECK-NEXT:    StoreToEnvironment r2, 159, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f160"
// CHECK-NEXT:    StoreToEnvironment r2, 160, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f161"
// CHECK-NEXT:    StoreToEnvironment r2, 161, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f162"
// CHECK-NEXT:    StoreToEnvironment r2, 162, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f163"
// CHECK-NEXT:    StoreToEnvironment r2, 163, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f164"
// CHECK-NEXT:    StoreToEnvironment r2, 164, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f165"
// CHECK-NEXT:    StoreToEnvironment r2, 165, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f166"
// CHECK-NEXT:    StoreToEnvironment r2, 166, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f167"
// CHECK-NEXT:    StoreToEnvironment r2, 167, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f168"
// CHECK-NEXT:    StoreToEnvironment r2, 168, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f169"
// CHECK-NEXT:    StoreToEnvironment r2, 169, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f170"
// CHECK-NEXT:    StoreToEnvironment r2, 170, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f171"
// CHECK-NEXT:    StoreToEnvironment r2, 171, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f172"
// CHECK-NEXT:    StoreToEnvironment r2, 172, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f173"
// CHECK-NEXT:    StoreToEnvironment r2, 173, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f174"
// CHECK-NEXT:    StoreToEnvironment r2, 174, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f175"
// CHECK-NEXT:    StoreToEnvironment r2, 175, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f176"
// CHECK-NEXT:    StoreToEnvironment r2, 176, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f177"
// CHECK-NEXT:    StoreToEnvironment r2, 177, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f178"
// CHECK-NEXT:    StoreToEnvironment r2, 178, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f179"
// CHECK-NEXT:    StoreToEnvironment r2, 179, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f180"
// CHECK-NEXT:    StoreToEnvironment r2, 180, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f181"
// CHECK-NEXT:    StoreToEnvironment r2, 181, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f182"
// CHECK-NEXT:    StoreToEnvironment r2, 182, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f183"
// CHECK-NEXT:    StoreToEnvironment r2, 183, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f184"
// CHECK-NEXT:    StoreToEnvironment r2, 184, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f185"
// CHECK-NEXT:    StoreToEnvironment r2, 185, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f186"
// CHECK-NEXT:    StoreToEnvironment r2, 186, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f187"
// CHECK-NEXT:    StoreToEnvironment r2, 187, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f188"
// CHECK-NEXT:    StoreToEnvironment r2, 188, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f189"
// CHECK-NEXT:    StoreToEnvironment r2, 189, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f190"
// CHECK-NEXT:    StoreToEnvironment r2, 190, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f191"
// CHECK-NEXT:    StoreToEnvironment r2, 191, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f192"
// CHECK-NEXT:    StoreToEnvironment r2, 192, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f193"
// CHECK-NEXT:    StoreToEnvironment r2, 193, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f194"
// CHECK-NEXT:    StoreToEnvironment r2, 194, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f195"
// CHECK-NEXT:    StoreToEnvironment r2, 195, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f196"
// CHECK-NEXT:    StoreToEnvironment r2, 196, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f197"
// CHECK-NEXT:    StoreToEnvironment r2, 197, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f198"
// CHECK-NEXT:    StoreToEnvironment r2, 198, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f199"
// CHECK-NEXT:    StoreToEnvironment r2, 199, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f200"
// CHECK-NEXT:    StoreToEnvironment r2, 200, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f201"
// CHECK-NEXT:    StoreToEnvironment r2, 201, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f202"
// CHECK-NEXT:    StoreToEnvironment r2, 202, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f203"
// CHECK-NEXT:    StoreToEnvironment r2, 203, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f204"
// CHECK-NEXT:    StoreToEnvironment r2, 204, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f205"
// CHECK-NEXT:    StoreToEnvironment r2, 205, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f206"
// CHECK-NEXT:    StoreToEnvironment r2, 206, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f207"
// CHECK-NEXT:    StoreToEnvironment r2, 207, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f208"
// CHECK-NEXT:    StoreToEnvironment r2, 208, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f209"
// CHECK-NEXT:    StoreToEnvironment r2, 209, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f210"
// CHECK-NEXT:    StoreToEnvironment r2, 210, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f211"
// CHECK-NEXT:    StoreToEnvironment r2, 211, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f212"
// CHECK-NEXT:    StoreToEnvironment r2, 212, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f213"
// CHECK-NEXT:    StoreToEnvironment r2, 213, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f214"
// CHECK-NEXT:    StoreToEnvironment r2, 214, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f215"
// CHECK-NEXT:    StoreToEnvironment r2, 215, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f216"
// CHECK-NEXT:    StoreToEnvironment r2, 216, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f217"
// CHECK-NEXT:    StoreToEnvironment r2, 217, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f218"
// CHECK-NEXT:    StoreToEnvironment r2, 218, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f219"
// CHECK-NEXT:    StoreToEnvironment r2, 219, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f220"
// CHECK-NEXT:    StoreToEnvironment r2, 220, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f221"
// CHECK-NEXT:    StoreToEnvironment r2, 221, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f222"
// CHECK-NEXT:    StoreToEnvironment r2, 222, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f223"
// CHECK-NEXT:    StoreToEnvironment r2, 223, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f224"
// CHECK-NEXT:    StoreToEnvironment r2, 224, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f225"
// CHECK-NEXT:    StoreToEnvironment r2, 225, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f226"
// CHECK-NEXT:    StoreToEnvironment r2, 226, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f227"
// CHECK-NEXT:    StoreToEnvironment r2, 227, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f228"
// CHECK-NEXT:    StoreToEnvironment r2, 228, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f229"
// CHECK-NEXT:    StoreToEnvironment r2, 229, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f230"
// CHECK-NEXT:    StoreToEnvironment r2, 230, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f231"
// CHECK-NEXT:    StoreToEnvironment r2, 231, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f232"
// CHECK-NEXT:    StoreToEnvironment r2, 232, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f233"
// CHECK-NEXT:    StoreToEnvironment r2, 233, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f234"
// CHECK-NEXT:    StoreToEnvironment r2, 234, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f235"
// CHECK-NEXT:    StoreToEnvironment r2, 235, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f236"
// CHECK-NEXT:    StoreToEnvironment r2, 236, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f237"
// CHECK-NEXT:    StoreToEnvironment r2, 237, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f238"
// CHECK-NEXT:    StoreToEnvironment r2, 238, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f239"
// CHECK-NEXT:    StoreToEnvironment r2, 239, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f240"
// CHECK-NEXT:    StoreToEnvironment r2, 240, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f241"
// CHECK-NEXT:    StoreToEnvironment r2, 241, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f242"
// CHECK-NEXT:    StoreToEnvironment r2, 242, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f243"
// CHECK-NEXT:    StoreToEnvironment r2, 243, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f244"
// CHECK-NEXT:    StoreToEnvironment r2, 244, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f245"
// CHECK-NEXT:    StoreToEnvironment r2, 245, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f246"
// CHECK-NEXT:    StoreToEnvironment r2, 246, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f247"
// CHECK-NEXT:    StoreToEnvironment r2, 247, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f248"
// CHECK-NEXT:    StoreToEnvironment r2, 248, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f249"
// CHECK-NEXT:    StoreToEnvironment r2, 249, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f250"
// CHECK-NEXT:    StoreToEnvironment r2, 250, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f251"
// CHECK-NEXT:    StoreToEnvironment r2, 251, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f252"
// CHECK-NEXT:    StoreToEnvironment r2, 252, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f253"
// CHECK-NEXT:    StoreToEnvironment r2, 253, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f254"
// CHECK-NEXT:    StoreToEnvironment r2, 254, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f255"
// CHECK-NEXT:    StoreToEnvironment r2, 255, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f256"
// CHECK-NEXT:    StoreToEnvironmentL r2, 256, r1
// CHECK-NEXT:    CreatePrivateName r1, "#f257"
// CHECK-NEXT:    StoreToEnvironmentL r2, 257, r1
// CHECK-NEXT:    CreateBaseClass   r1, r3, r2, 1
// CHECK-NEXT:    Mov               r4, r3
// CHECK-NEXT:    LoadConstString   r3, "sum"
// CHECK-NEXT:    CreateClosure     r2, r2, NCFunction<sum>
// CHECK-NEXT:    DefineOwnByVal    r4, r2, r3, 0
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    Construct         r3, r1, 1
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    TryGetById        r2, r1, 0, "print"
// CHECK-NEXT:    GetByIdShort      r1, r3, 1, "sum"
// CHECK-NEXT:    Call1             r1, r1, r3
// CHECK-NEXT:    Call2             r1, r2, r0, r1
// CHECK-NEXT:    Ret               r0

// CHECK:Constructor<ManyPrivateProperties>(1 params, 14 registers, 1 numbers, 1 non-pointers):
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    GetNewTarget      r2
// CHECK-NEXT:    GetById           r2, r2, 0, "prototype"
// CHECK-NEXT:    NewObjectWithParent r2, r2
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 0
// CHECK-NEXT:    PrivateIsIn       r1, r4, r2, r0
// CHECK-NEXT:    JmpTrueLong       L1, r1
// CHECK-NEXT:    LoadConstUInt8    r0, 10
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 1
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 2
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 3
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 4
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 5
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 6
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 7
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 8
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 9
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 10
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 11
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 12
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 13
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 14
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 15
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 16
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 17
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 18
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 19
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 20
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 21
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 22
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 23
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 24
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 25
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 26
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 27
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 28
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 29
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 30
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 31
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 32
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 33
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 34
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 35
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 36
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 37
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 38
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 39
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 40
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 41
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 42
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 43
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 44
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 45
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 46
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 47
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 48
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 49
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 50
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 51
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 52
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 53
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 54
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 55
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 56
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 57
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 58
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 59
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 60
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 61
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 62
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 63
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 64
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 65
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 66
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 67
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 68
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 69
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 70
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 71
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 72
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 73
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 74
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 75
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 76
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 77
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 78
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 79
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 80
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 81
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 82
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 83
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 84
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 85
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 86
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 87
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 88
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 89
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 90
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 91
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 92
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 93
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 94
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 95
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 96
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 97
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 98
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 99
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 100
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 101
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 102
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 103
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 104
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 105
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 106
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 107
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 108
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 109
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 110
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 111
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 112
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 113
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 114
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 115
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 116
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 117
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 118
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 119
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 120
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 121
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 122
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 123
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 124
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 125
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 126
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 127
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 128
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 129
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 130
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 131
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 132
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 133
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 134
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 135
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 136
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 137
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 138
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 139
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 140
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 141
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 142
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 143
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 144
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 145
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 146
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 147
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 148
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 149
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 150
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 151
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 152
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 153
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 154
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 155
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 156
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 157
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 158
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 159
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 160
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 161
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 162
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 163
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 164
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 165
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 166
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 167
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 168
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 169
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 170
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 171
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 172
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 173
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 174
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 175
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 176
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 177
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 178
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 179
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 180
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 181
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 182
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 183
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 184
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 185
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 186
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 187
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 188
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 189
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 190
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 191
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 192
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 193
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 194
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 195
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 196
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 197
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 198
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 199
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 200
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 201
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 202
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 203
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 204
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 205
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 206
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 207
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 208
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 209
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 210
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 211
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 212
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 213
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 214
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 215
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 216
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 217
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 218
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 219
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 220
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 221
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 222
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 223
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 224
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 225
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 226
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 227
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 228
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 229
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 230
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 231
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 232
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 233
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 234
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 235
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 236
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 237
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 238
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 239
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 240
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 241
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 242
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 243
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 244
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 245
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 246
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 247
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 248
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 249
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 250
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 251
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 252
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 253
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 254
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironment r4, r3, 255
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironmentL r4, r3, 256
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r4
// CHECK-NEXT:    LoadFromEnvironmentL r3, r3, 257
// CHECK-NEXT:    AddOwnPrivateBySym r2, r0, r3
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstString   r5, "Cannot initialize"...
// CHECK-NEXT:    CallBuiltin       r2, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:NCFunction<sum>(1 params, 4 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0018, lexical 0x0000
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 0
// CHECK-NEXT:    GetOwnPrivateBySym r3, r2, 0, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 1
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 1, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 2
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 2, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 3
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 3, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 4
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 4, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 5
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 5, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 6
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 6, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 7
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 7, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 8
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 8, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 9
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 9, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 10
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 10, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 11
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 11, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 12
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 12, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 13
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 13, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 14
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 14, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 15
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 15, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 16
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 16, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 17
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 17, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 18
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 18, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 19
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 19, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 20
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 20, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 21
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 21, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 22
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 22, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 23
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 23, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 24
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 24, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 25
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 25, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 26
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 26, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 27
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 27, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 28
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 28, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 29
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 29, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 30
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 30, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 31
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 31, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 32
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 32, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 33
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 33, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 34
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 34, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 35
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 35, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 36
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 36, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 37
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 37, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 38
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 38, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 39
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 39, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 40
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 40, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 41
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 41, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 42
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 42, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 43
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 43, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 44
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 44, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 45
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 45, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 46
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 46, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 47
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 47, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 48
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 48, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 49
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 49, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 50
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 50, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 51
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 51, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 52
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 52, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 53
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 53, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 54
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 54, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 55
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 55, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 56
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 56, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 57
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 57, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 58
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 58, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 59
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 59, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 60
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 60, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 61
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 61, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 62
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 62, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 63
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 63, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 64
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 64, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 65
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 65, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 66
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 66, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 67
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 67, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 68
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 68, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 69
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 69, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 70
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 70, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 71
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 71, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 72
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 72, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 73
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 73, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 74
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 74, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 75
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 75, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 76
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 76, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 77
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 77, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 78
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 78, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 79
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 79, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 80
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 80, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 81
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 81, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 82
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 82, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 83
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 83, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 84
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 84, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 85
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 85, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 86
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 86, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 87
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 87, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 88
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 88, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 89
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 89, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 90
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 90, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 91
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 91, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 92
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 92, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 93
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 93, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 94
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 94, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 95
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 95, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 96
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 96, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 97
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 97, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 98
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 98, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 99
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 99, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 100
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 100, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 101
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 101, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 102
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 102, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 103
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 103, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 104
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 104, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 105
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 105, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 106
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 106, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 107
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 107, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 108
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 108, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 109
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 109, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 110
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 110, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 111
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 111, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 112
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 112, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 113
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 113, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 114
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 114, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 115
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 115, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 116
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 116, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 117
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 117, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 118
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 118, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 119
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 119, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 120
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 120, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 121
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 121, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 122
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 122, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 123
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 123, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 124
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 124, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 125
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 125, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 126
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 126, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 127
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 127, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 128
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 128, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 129
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 129, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 130
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 130, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 131
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 131, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 132
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 132, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 133
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 133, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 134
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 134, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 135
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 135, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 136
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 136, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 137
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 137, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 138
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 138, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 139
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 139, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 140
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 140, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 141
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 141, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 142
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 142, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 143
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 143, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 144
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 144, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 145
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 145, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 146
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 146, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 147
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 147, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 148
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 148, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 149
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 149, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 150
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 150, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 151
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 151, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 152
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 152, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 153
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 153, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 154
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 154, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 155
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 155, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 156
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 156, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 157
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 157, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 158
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 158, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 159
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 159, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 160
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 160, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 161
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 161, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 162
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 162, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 163
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 163, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 164
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 164, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 165
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 165, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 166
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 166, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 167
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 167, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 168
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 168, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 169
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 169, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 170
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 170, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 171
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 171, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 172
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 172, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 173
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 173, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 174
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 174, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 175
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 175, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 176
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 176, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 177
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 177, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 178
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 178, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 179
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 179, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 180
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 180, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 181
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 181, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 182
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 182, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 183
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 183, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 184
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 184, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 185
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 185, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 186
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 186, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 187
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 187, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 188
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 188, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 189
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 189, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 190
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 190, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 191
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 191, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 192
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 192, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 193
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 193, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 194
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 194, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 195
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 195, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 196
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 196, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 197
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 197, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 198
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 198, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 199
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 199, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 200
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 200, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 201
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 201, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 202
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 202, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 203
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 203, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 204
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 204, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 205
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 205, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 206
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 206, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 207
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 207, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 208
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 208, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 209
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 209, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 210
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 210, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 211
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 211, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 212
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 212, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 213
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 213, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 214
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 214, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 215
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 215, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 216
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 216, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 217
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 217, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 218
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 218, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 219
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 219, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 220
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 220, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 221
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 221, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 222
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 222, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 223
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 223, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 224
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 224, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 225
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 225, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 226
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 226, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 227
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 227, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 228
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 228, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 229
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 229, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 230
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 230, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 231
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 231, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 232
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 232, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 233
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 233, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 234
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 234, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 235
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 235, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 236
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 236, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 237
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 237, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 238
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 238, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 239
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 239, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 240
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 240, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 241
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 241, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 242
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 242, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 243
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 243, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 244
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 244, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 245
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 245, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 246
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 246, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 247
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 247, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 248
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 248, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 249
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 249, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 250
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 250, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 251
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 251, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 252
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 252, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 253
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 253, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 254
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 254, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r0, 255
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 255, r1
// CHECK-NEXT:    Add               r3, r3, r1
// CHECK-NEXT:    LoadFromEnvironmentL r1, r0, 256
// CHECK-NEXT:    GetOwnPrivateBySym r1, r2, 255, r1
// CHECK-NEXT:    Add               r1, r3, r1
// CHECK-NEXT:    LoadFromEnvironmentL r0, r0, 257
// CHECK-NEXT:    GetOwnPrivateBySym r0, r2, 255, r0
// CHECK-NEXT:    Add               r0, r1, r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}private-property-cache-ids.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2606: line 11 col 3
// CHECK-NEXT:    bc 2615: line 132 col 38
// CHECK-NEXT:    bc 2621: line 133 col 3
// CHECK-NEXT:    bc 2627: line 133 col 16
// CHECK-NEXT:    bc 2632: line 133 col 16
// CHECK-NEXT:    bc 2636: line 133 col 8
// CHECK-NEXT:  0x0018  function idx 2, starts at line 70 col 5
// CHECK-NEXT:    bc 10: line 74 col 15
// CHECK-NEXT:    bc 19: line 74 col 26
// CHECK-NEXT:    bc 24: line 74 col 11
// CHECK-NEXT:    bc 32: line 74 col 37
// CHECK-NEXT:    bc 37: line 74 col 11
// CHECK-NEXT:    bc 45: line 74 col 48
// CHECK-NEXT:    bc 50: line 74 col 11
// CHECK-NEXT:    bc 58: line 74 col 59
// CHECK-NEXT:    bc 63: line 74 col 11
// CHECK-NEXT:    bc 71: line 75 col 15
// CHECK-NEXT:    bc 76: line 74 col 11
// CHECK-NEXT:    bc 84: line 75 col 26
// CHECK-NEXT:    bc 89: line 74 col 11
// CHECK-NEXT:    bc 97: line 75 col 37
// CHECK-NEXT:    bc 102: line 74 col 11
// CHECK-NEXT:    bc 110: line 75 col 48
// CHECK-NEXT:    bc 115: line 74 col 11
// CHECK-NEXT:    bc 123: line 75 col 59
// CHECK-NEXT:    bc 128: line 74 col 11
// CHECK-NEXT:    bc 136: line 76 col 15
// CHECK-NEXT:    bc 141: line 74 col 11
// CHECK-NEXT:    bc 149: line 76 col 27
// CHECK-NEXT:    bc 154: line 74 col 11
// CHECK-NEXT:    bc 162: line 76 col 39
// CHECK-NEXT:    bc 167: line 74 col 11
// CHECK-NEXT:    bc 175: line 76 col 51
// CHECK-NEXT:    bc 180: line 74 col 11
// CHECK-NEXT:    bc 188: line 76 col 63
// CHECK-NEXT:    bc 193: line 74 col 11
// CHECK-NEXT:    bc 201: line 77 col 15
// CHECK-NEXT:    bc 206: line 74 col 11
// CHECK-NEXT:    bc 214: line 77 col 27
// CHECK-NEXT:    bc 219: line 74 col 11
// CHECK-NEXT:    bc 227: line 77 col 39
// CHECK-NEXT:    bc 232: line 74 col 11
// CHECK-NEXT:    bc 240: line 77 col 51
// CHECK-NEXT:    bc 245: line 74 col 11
// CHECK-NEXT:    bc 253: line 77 col 63
// CHECK-NEXT:    bc 258: line 74 col 11
// CHECK-NEXT:    bc 266: line 78 col 15
// CHECK-NEXT:    bc 271: line 74 col 11
// CHECK-NEXT:    bc 279: line 78 col 27
// CHECK-NEXT:    bc 284: line 74 col 11
// CHECK-NEXT:    bc 292: line 78 col 39
// CHECK-NEXT:    bc 297: line 74 col 11
// CHECK-NEXT:    bc 305: line 78 col 51
// CHECK-NEXT:    bc 310: line 74 col 11
// CHECK-NEXT:    bc 318: line 78 col 63
// CHECK-NEXT:    bc 323: line 74 col 11
// CHECK-NEXT:    bc 331: line 79 col 15
// CHECK-NEXT:    bc 336: line 74 col 11
// CHECK-NEXT:    bc 344: line 79 col 27
// CHECK-NEXT:    bc 349: line 74 col 11
// CHECK-NEXT:    bc 357: line 79 col 39
// CHECK-NEXT:    bc 362: line 74 col 11
// CHECK-NEXT:    bc 370: line 79 col 51
// CHECK-NEXT:    bc 375: line 74 col 11
// CHECK-NEXT:    bc 383: line 79 col 63
// CHECK-NEXT:    bc 388: line 74 col 11
// CHECK-NEXT:    bc 396: line 80 col 15
// CHECK-NEXT:    bc 401: line 74 col 11
// CHECK-NEXT:    bc 409: line 80 col 27
// CHECK-NEXT:    bc 414: line 74 col 11
// CHECK-NEXT:    bc 422: line 80 col 39
// CHECK-NEXT:    bc 427: line 74 col 11
// CHECK-NEXT:    bc 435: line 80 col 51
// CHECK-NEXT:    bc 440: line 74 col 11
// CHECK-NEXT:    bc 448: line 80 col 63
// CHECK-NEXT:    bc 453: line 74 col 11
// CHECK-NEXT:    bc 461: line 81 col 15
// CHECK-NEXT:    bc 466: line 74 col 11
// CHECK-NEXT:    bc 474: line 81 col 27
// CHECK-NEXT:    bc 479: line 74 col 11
// CHECK-NEXT:    bc 487: line 81 col 39
// CHECK-NEXT:    bc 492: line 74 col 11
// CHECK-NEXT:    bc 500: line 81 col 51
// CHECK-NEXT:    bc 505: line 74 col 11
// CHECK-NEXT:    bc 513: line 81 col 63
// CHECK-NEXT:    bc 518: line 74 col 11
// CHECK-NEXT:    bc 526: line 82 col 15
// CHECK-NEXT:    bc 531: line 74 col 11
// CHECK-NEXT:    bc 539: line 82 col 27
// CHECK-NEXT:    bc 544: line 74 col 11
// CHECK-NEXT:    bc 552: line 82 col 39
// CHECK-NEXT:    bc 557: line 74 col 11
// CHECK-NEXT:    bc 565: line 82 col 51
// CHECK-NEXT:    bc 570: line 74 col 11
// CHECK-NEXT:    bc 578: line 82 col 63
// CHECK-NEXT:    bc 583: line 74 col 11
// CHECK-NEXT:    bc 591: line 83 col 15
// CHECK-NEXT:    bc 596: line 74 col 11
// CHECK-NEXT:    bc 604: line 83 col 27
// CHECK-NEXT:    bc 609: line 74 col 11
// CHECK-NEXT:    bc 617: line 83 col 39
// CHECK-NEXT:    bc 622: line 74 col 11
// CHECK-NEXT:    bc 630: line 83 col 51
// CHECK-NEXT:    bc 635: line 74 col 11
// CHECK-NEXT:    bc 643: line 83 col 63
// CHECK-NEXT:    bc 648: line 74 col 11
// CHECK-NEXT:    bc 656: line 84 col 15
// CHECK-NEXT:    bc 661: line 74 col 11
// CHECK-NEXT:    bc 669: line 84 col 27
// CHECK-NEXT:    bc 674: line 74 col 11
// CHECK-NEXT:    bc 682: line 84 col 39
// CHECK-NEXT:    bc 687: line 74 col 11
// CHECK-NEXT:    bc 695: line 84 col 51
// CHECK-NEXT:    bc 700: line 74 col 11
// CHECK-NEXT:    bc 708: line 84 col 63
// CHECK-NEXT:    bc 713: line 74 col 11
// CHECK-NEXT:    bc 721: line 85 col 15
// CHECK-NEXT:    bc 726: line 74 col 11
// CHECK-NEXT:    bc 734: line 85 col 27
// CHECK-NEXT:    bc 739: line 74 col 11
// CHECK-NEXT:    bc 747: line 85 col 39
// CHECK-NEXT:    bc 752: line 74 col 11
// CHECK-NEXT:    bc 760: line 85 col 51
// CHECK-NEXT:    bc 765: line 74 col 11
// CHECK-NEXT:    bc 773: line 85 col 63
// CHECK-NEXT:    bc 778: line 74 col 11
// CHECK-NEXT:    bc 786: line 86 col 15
// CHECK-NEXT:    bc 791: line 74 col 11
// CHECK-NEXT:    bc 799: line 86 col 27
// CHECK-NEXT:    bc 804: line 74 col 11
// CHECK-NEXT:    bc 812: line 86 col 39
// CHECK-NEXT:    bc 817: line 74 col 11
// CHECK-NEXT:    bc 825: line 86 col 51
// CHECK-NEXT:    bc 830: line 74 col 11
// CHECK-NEXT:    bc 838: line 86 col 63
// CHECK-NEXT:    bc 843: line 74 col 11
// CHECK-NEXT:    bc 851: line 87 col 15
// CHECK-NEXT:    bc 856: line 74 col 11
// CHECK-NEXT:    bc 864: line 87 col 27
// CHECK-NEXT:    bc 869: line 74 col 11
// CHECK-NEXT:    bc 877: line 87 col 39
// CHECK-NEXT:    bc 882: line 74 col 11
// CHECK-NEXT:    bc 890: line 87 col 51
// CHECK-NEXT:    bc 895: line 74 col 11
// CHECK-NEXT:    bc 903: line 87 col 63
// CHECK-NEXT:    bc 908: line 74 col 11
// CHECK-NEXT:    bc 916: line 88 col 15
// CHECK-NEXT:    bc 921: line 74 col 11
// CHECK-NEXT:    bc 929: line 88 col 27
// CHECK-NEXT:    bc 934: line 74 col 11
// CHECK-NEXT:    bc 942: line 88 col 39
// CHECK-NEXT:    bc 947: line 74 col 11
// CHECK-NEXT:    bc 955: line 88 col 51
// CHECK-NEXT:    bc 960: line 74 col 11
// CHECK-NEXT:    bc 968: line 88 col 63
// CHECK-NEXT:    bc 973: line 74 col 11
// CHECK-NEXT:    bc 981: line 89 col 15
// CHECK-NEXT:    bc 986: line 74 col 11
// CHECK-NEXT:    bc 994: line 89 col 27
// CHECK-NEXT:    bc 999: line 74 col 11
// CHECK-NEXT:    bc 1007: line 89 col 39
// CHECK-NEXT:    bc 1012: line 74 col 11
// CHECK-NEXT:    bc 1020: line 89 col 51
// CHECK-NEXT:    bc 1025: line 74 col 11
// CHECK-NEXT:    bc 1033: line 89 col 63
// CHECK-NEXT:    bc 1038: line 74 col 11
// CHECK-NEXT:    bc 1046: line 90 col 15
// CHECK-NEXT:    bc 1051: line 74 col 11
// CHECK-NEXT:    bc 1059: line 90 col 27
// CHECK-NEXT:    bc 1064: line 74 col 11
// CHECK-NEXT:    bc 1072: line 90 col 39
// CHECK-NEXT:    bc 1077: line 74 col 11
// CHECK-NEXT:    bc 1085: line 90 col 51
// CHECK-NEXT:    bc 1090: line 74 col 11
// CHECK-NEXT:    bc 1098: line 90 col 63
// CHECK-NEXT:    bc 1103: line 74 col 11
// CHECK-NEXT:    bc 1111: line 91 col 15
// CHECK-NEXT:    bc 1116: line 74 col 11
// CHECK-NEXT:    bc 1124: line 91 col 27
// CHECK-NEXT:    bc 1129: line 74 col 11
// CHECK-NEXT:    bc 1137: line 91 col 39
// CHECK-NEXT:    bc 1142: line 74 col 11
// CHECK-NEXT:    bc 1150: line 91 col 51
// CHECK-NEXT:    bc 1155: line 74 col 11
// CHECK-NEXT:    bc 1163: line 91 col 63
// CHECK-NEXT:    bc 1168: line 74 col 11
// CHECK-NEXT:    bc 1176: line 92 col 15
// CHECK-NEXT:    bc 1181: line 74 col 11
// CHECK-NEXT:    bc 1189: line 92 col 27
// CHECK-NEXT:    bc 1194: line 74 col 11
// CHECK-NEXT:    bc 1202: line 92 col 39
// CHECK-NEXT:    bc 1207: line 74 col 11
// CHECK-NEXT:    bc 1215: line 92 col 51
// CHECK-NEXT:    bc 1220: line 74 col 11
// CHECK-NEXT:    bc 1228: line 92 col 63
// CHECK-NEXT:    bc 1233: line 74 col 11
// CHECK-NEXT:    bc 1241: line 93 col 15
// CHECK-NEXT:    bc 1246: line 74 col 11
// CHECK-NEXT:    bc 1254: line 93 col 27
// CHECK-NEXT:    bc 1259: line 74 col 11
// CHECK-NEXT:    bc 1267: line 93 col 39
// CHECK-NEXT:    bc 1272: line 74 col 11
// CHECK-NEXT:    bc 1280: line 93 col 51
// CHECK-NEXT:    bc 1285: line 74 col 11
// CHECK-NEXT:    bc 1293: line 93 col 63
// CHECK-NEXT:    bc 1298: line 74 col 11
// CHECK-NEXT:    bc 1306: line 95 col 15
// CHECK-NEXT:    bc 1311: line 74 col 11
// CHECK-NEXT:    bc 1319: line 95 col 28
// CHECK-NEXT:    bc 1324: line 74 col 11
// CHECK-NEXT:    bc 1332: line 95 col 41
// CHECK-NEXT:    bc 1337: line 74 col 11
// CHECK-NEXT:    bc 1345: line 95 col 54
// CHECK-NEXT:    bc 1350: line 74 col 11
// CHECK-NEXT:    bc 1358: line 95 col 67
// CHECK-NEXT:    bc 1363: line 74 col 11
// CHECK-NEXT:    bc 1371: line 96 col 15
// CHECK-NEXT:    bc 1376: line 74 col 11
// CHECK-NEXT:    bc 1384: line 96 col 28
// CHECK-NEXT:    bc 1389: line 74 col 11
// CHECK-NEXT:    bc 1397: line 96 col 41
// CHECK-NEXT:    bc 1402: line 74 col 11
// CHECK-NEXT:    bc 1410: line 96 col 54
// CHECK-NEXT:    bc 1415: line 74 col 11
// CHECK-NEXT:    bc 1423: line 96 col 67
// CHECK-NEXT:    bc 1428: line 74 col 11
// CHECK-NEXT:    bc 1436: line 97 col 15
// CHECK-NEXT:    bc 1441: line 74 col 11
// CHECK-NEXT:    bc 1449: line 97 col 28
// CHECK-NEXT:    bc 1454: line 74 col 11
// CHECK-NEXT:    bc 1462: line 97 col 41
// CHECK-NEXT:    bc 1467: line 74 col 11
// CHECK-NEXT:    bc 1475: line 97 col 54
// CHECK-NEXT:    bc 1480: line 74 col 11
// CHECK-NEXT:    bc 1488: line 97 col 67
// CHECK-NEXT:    bc 1493: line 74 col 11
// CHECK-NEXT:    bc 1501: line 98 col 15
// CHECK-NEXT:    bc 1506: line 74 col 11
// CHECK-NEXT:    bc 1514: line 98 col 28
// CHECK-NEXT:    bc 1519: line 74 col 11
// CHECK-NEXT:    bc 1527: line 98 col 41
// CHECK-NEXT:    bc 1532: line 74 col 11
// CHECK-NEXT:    bc 1540: line 98 col 54
// CHECK-NEXT:    bc 1545: line 74 col 11
// CHECK-NEXT:    bc 1553: line 98 col 67
// CHECK-NEXT:    bc 1558: line 74 col 11
// CHECK-NEXT:    bc 1566: line 99 col 15
// CHECK-NEXT:    bc 1571: line 74 col 11
// CHECK-NEXT:    bc 1579: line 99 col 28
// CHECK-NEXT:    bc 1584: line 74 col 11
// CHECK-NEXT:    bc 1592: line 99 col 41
// CHECK-NEXT:    bc 1597: line 74 col 11
// CHECK-NEXT:    bc 1605: line 99 col 54
// CHECK-NEXT:    bc 1610: line 74 col 11
// CHECK-NEXT:    bc 1618: line 99 col 67
// CHECK-NEXT:    bc 1623: line 74 col 11
// CHECK-NEXT:    bc 1631: line 100 col 15
// CHECK-NEXT:    bc 1636: line 74 col 11
// CHECK-NEXT:    bc 1644: line 100 col 28
// CHECK-NEXT:    bc 1649: line 74 col 11
// CHECK-NEXT:    bc 1657: line 100 col 41
// CHECK-NEXT:    bc 1662: line 74 col 11
// CHECK-NEXT:    bc 1670: line 100 col 54
// CHECK-NEXT:    bc 1675: line 74 col 11
// CHECK-NEXT:    bc 1683: line 100 col 67
// CHECK-NEXT:    bc 1688: line 74 col 11
// CHECK-NEXT:    bc 1696: line 101 col 15
// CHECK-NEXT:    bc 1701: line 74 col 11
// CHECK-NEXT:    bc 1709: line 101 col 28
// CHECK-NEXT:    bc 1714: line 74 col 11
// CHECK-NEXT:    bc 1722: line 101 col 41
// CHECK-NEXT:    bc 1727: line 74 col 11
// CHECK-NEXT:    bc 1735: line 101 col 54
// CHECK-NEXT:    bc 1740: line 74 col 11
// CHECK-NEXT:    bc 1748: line 101 col 67
// CHECK-NEXT:    bc 1753: line 74 col 11
// CHECK-NEXT:    bc 1761: line 102 col 15
// CHECK-NEXT:    bc 1766: line 74 col 11
// CHECK-NEXT:    bc 1774: line 102 col 28
// CHECK-NEXT:    bc 1779: line 74 col 11
// CHECK-NEXT:    bc 1787: line 102 col 41
// CHECK-NEXT:    bc 1792: line 74 col 11
// CHECK-NEXT:    bc 1800: line 102 col 54
// CHECK-NEXT:    bc 1805: line 74 col 11
// CHECK-NEXT:    bc 1813: line 102 col 67
// CHECK-NEXT:    bc 1818: line 74 col 11
// CHECK-NEXT:    bc 1826: line 103 col 15
// CHECK-NEXT:    bc 1831: line 74 col 11
// CHECK-NEXT:    bc 1839: line 103 col 28
// CHECK-NEXT:    bc 1844: line 74 col 11
// CHECK-NEXT:    bc 1852: line 103 col 41
// CHECK-NEXT:    bc 1857: line 74 col 11
// CHECK-NEXT:    bc 1865: line 103 col 54
// CHECK-NEXT:    bc 1870: line 74 col 11
// CHECK-NEXT:    bc 1878: line 103 col 67
// CHECK-NEXT:    bc 1883: line 74 col 11
// CHECK-NEXT:    bc 1891: line 104 col 15
// CHECK-NEXT:    bc 1896: line 74 col 11
// CHECK-NEXT:    bc 1904: line 104 col 28
// CHECK-NEXT:    bc 1909: line 74 col 11
// CHECK-NEXT:    bc 1917: line 104 col 41
// CHECK-NEXT:    bc 1922: line 74 col 11
// CHECK-NEXT:    bc 1930: line 104 col 54
// CHECK-NEXT:    bc 1935: line 74 col 11
// CHECK-NEXT:    bc 1943: line 104 col 67
// CHECK-NEXT:    bc 1948: line 74 col 11
// CHECK-NEXT:    bc 1956: line 105 col 15
// CHECK-NEXT:    bc 1961: line 74 col 11
// CHECK-NEXT:    bc 1969: line 105 col 28
// CHECK-NEXT:    bc 1974: line 74 col 11
// CHECK-NEXT:    bc 1982: line 105 col 41
// CHECK-NEXT:    bc 1987: line 74 col 11
// CHECK-NEXT:    bc 1995: line 105 col 54
// CHECK-NEXT:    bc 2000: line 74 col 11
// CHECK-NEXT:    bc 2008: line 105 col 67
// CHECK-NEXT:    bc 2013: line 74 col 11
// CHECK-NEXT:    bc 2021: line 106 col 15
// CHECK-NEXT:    bc 2026: line 74 col 11
// CHECK-NEXT:    bc 2034: line 106 col 28
// CHECK-NEXT:    bc 2039: line 74 col 11
// CHECK-NEXT:    bc 2047: line 106 col 41
// CHECK-NEXT:    bc 2052: line 74 col 11
// CHECK-NEXT:    bc 2060: line 106 col 54
// CHECK-NEXT:    bc 2065: line 74 col 11
// CHECK-NEXT:    bc 2073: line 106 col 67
// CHECK-NEXT:    bc 2078: line 74 col 11
// CHECK-NEXT:    bc 2086: line 107 col 15
// CHECK-NEXT:    bc 2091: line 74 col 11
// CHECK-NEXT:    bc 2099: line 107 col 28
// CHECK-NEXT:    bc 2104: line 74 col 11
// CHECK-NEXT:    bc 2112: line 107 col 41
// CHECK-NEXT:    bc 2117: line 74 col 11
// CHECK-NEXT:    bc 2125: line 107 col 54
// CHECK-NEXT:    bc 2130: line 74 col 11
// CHECK-NEXT:    bc 2138: line 107 col 67
// CHECK-NEXT:    bc 2143: line 74 col 11
// CHECK-NEXT:    bc 2151: line 108 col 15
// CHECK-NEXT:    bc 2156: line 74 col 11
// CHECK-NEXT:    bc 2164: line 108 col 28
// CHECK-NEXT:    bc 2169: line 74 col 11
// CHECK-NEXT:    bc 2177: line 108 col 41
// CHECK-NEXT:    bc 2182: line 74 col 11
// CHECK-NEXT:    bc 2190: line 108 col 54
// CHECK-NEXT:    bc 2195: line 74 col 11
// CHECK-NEXT:    bc 2203: line 108 col 67
// CHECK-NEXT:    bc 2208: line 74 col 11
// CHECK-NEXT:    bc 2216: line 109 col 15
// CHECK-NEXT:    bc 2221: line 74 col 11
// CHECK-NEXT:    bc 2229: line 109 col 28
// CHECK-NEXT:    bc 2234: line 74 col 11
// CHECK-NEXT:    bc 2242: line 109 col 41
// CHECK-NEXT:    bc 2247: line 74 col 11
// CHECK-NEXT:    bc 2255: line 109 col 54
// CHECK-NEXT:    bc 2260: line 74 col 11
// CHECK-NEXT:    bc 2268: line 109 col 67
// CHECK-NEXT:    bc 2273: line 74 col 11
// CHECK-NEXT:    bc 2281: line 110 col 15
// CHECK-NEXT:    bc 2286: line 74 col 11
// CHECK-NEXT:    bc 2294: line 110 col 28
// CHECK-NEXT:    bc 2299: line 74 col 11
// CHECK-NEXT:    bc 2307: line 110 col 41
// CHECK-NEXT:    bc 2312: line 74 col 11
// CHECK-NEXT:    bc 2320: line 110 col 54
// CHECK-NEXT:    bc 2325: line 74 col 11
// CHECK-NEXT:    bc 2333: line 110 col 67
// CHECK-NEXT:    bc 2338: line 74 col 11
// CHECK-NEXT:    bc 2346: line 111 col 15
// CHECK-NEXT:    bc 2351: line 74 col 11
// CHECK-NEXT:    bc 2359: line 111 col 28
// CHECK-NEXT:    bc 2364: line 74 col 11
// CHECK-NEXT:    bc 2372: line 111 col 41
// CHECK-NEXT:    bc 2377: line 74 col 11
// CHECK-NEXT:    bc 2385: line 111 col 54
// CHECK-NEXT:    bc 2390: line 74 col 11
// CHECK-NEXT:    bc 2398: line 111 col 67
// CHECK-NEXT:    bc 2403: line 74 col 11
// CHECK-NEXT:    bc 2411: line 112 col 15
// CHECK-NEXT:    bc 2416: line 74 col 11
// CHECK-NEXT:    bc 2424: line 112 col 28
// CHECK-NEXT:    bc 2429: line 74 col 11
// CHECK-NEXT:    bc 2437: line 112 col 41
// CHECK-NEXT:    bc 2442: line 74 col 11
// CHECK-NEXT:    bc 2450: line 112 col 54
// CHECK-NEXT:    bc 2455: line 74 col 11
// CHECK-NEXT:    bc 2463: line 112 col 67
// CHECK-NEXT:    bc 2468: line 74 col 11
// CHECK-NEXT:    bc 2476: line 113 col 15
// CHECK-NEXT:    bc 2481: line 74 col 11
// CHECK-NEXT:    bc 2489: line 113 col 28
// CHECK-NEXT:    bc 2494: line 74 col 11
// CHECK-NEXT:    bc 2502: line 113 col 41
// CHECK-NEXT:    bc 2507: line 74 col 11
// CHECK-NEXT:    bc 2515: line 113 col 54
// CHECK-NEXT:    bc 2520: line 74 col 11
// CHECK-NEXT:    bc 2528: line 113 col 67
// CHECK-NEXT:    bc 2533: line 74 col 11
// CHECK-NEXT:    bc 2541: line 114 col 15
// CHECK-NEXT:    bc 2546: line 74 col 11
// CHECK-NEXT:    bc 2554: line 114 col 28
// CHECK-NEXT:    bc 2559: line 74 col 11
// CHECK-NEXT:    bc 2567: line 114 col 41
// CHECK-NEXT:    bc 2572: line 74 col 11
// CHECK-NEXT:    bc 2580: line 114 col 54
// CHECK-NEXT:    bc 2585: line 74 col 11
// CHECK-NEXT:    bc 2593: line 114 col 67
// CHECK-NEXT:    bc 2598: line 74 col 11
// CHECK-NEXT:    bc 2606: line 116 col 15
// CHECK-NEXT:    bc 2611: line 74 col 11
// CHECK-NEXT:    bc 2619: line 116 col 28
// CHECK-NEXT:    bc 2624: line 74 col 11
// CHECK-NEXT:    bc 2632: line 116 col 41
// CHECK-NEXT:    bc 2637: line 74 col 11
// CHECK-NEXT:    bc 2645: line 116 col 54
// CHECK-NEXT:    bc 2650: line 74 col 11
// CHECK-NEXT:    bc 2658: line 116 col 67
// CHECK-NEXT:    bc 2663: line 74 col 11
// CHECK-NEXT:    bc 2671: line 117 col 15
// CHECK-NEXT:    bc 2676: line 74 col 11
// CHECK-NEXT:    bc 2684: line 117 col 28
// CHECK-NEXT:    bc 2689: line 74 col 11
// CHECK-NEXT:    bc 2697: line 117 col 41
// CHECK-NEXT:    bc 2702: line 74 col 11
// CHECK-NEXT:    bc 2710: line 117 col 54
// CHECK-NEXT:    bc 2715: line 74 col 11
// CHECK-NEXT:    bc 2723: line 117 col 67
// CHECK-NEXT:    bc 2728: line 74 col 11
// CHECK-NEXT:    bc 2736: line 118 col 15
// CHECK-NEXT:    bc 2741: line 74 col 11
// CHECK-NEXT:    bc 2749: line 118 col 28
// CHECK-NEXT:    bc 2754: line 74 col 11
// CHECK-NEXT:    bc 2762: line 118 col 41
// CHECK-NEXT:    bc 2767: line 74 col 11
// CHECK-NEXT:    bc 2775: line 118 col 54
// CHECK-NEXT:    bc 2780: line 74 col 11
// CHECK-NEXT:    bc 2788: line 118 col 67
// CHECK-NEXT:    bc 2793: line 74 col 11
// CHECK-NEXT:    bc 2801: line 119 col 15
// CHECK-NEXT:    bc 2806: line 74 col 11
// CHECK-NEXT:    bc 2814: line 119 col 28
// CHECK-NEXT:    bc 2819: line 74 col 11
// CHECK-NEXT:    bc 2827: line 119 col 41
// CHECK-NEXT:    bc 2832: line 74 col 11
// CHECK-NEXT:    bc 2840: line 119 col 54
// CHECK-NEXT:    bc 2845: line 74 col 11
// CHECK-NEXT:    bc 2853: line 119 col 67
// CHECK-NEXT:    bc 2858: line 74 col 11
// CHECK-NEXT:    bc 2866: line 120 col 15
// CHECK-NEXT:    bc 2871: line 74 col 11
// CHECK-NEXT:    bc 2879: line 120 col 28
// CHECK-NEXT:    bc 2884: line 74 col 11
// CHECK-NEXT:    bc 2892: line 120 col 41
// CHECK-NEXT:    bc 2897: line 74 col 11
// CHECK-NEXT:    bc 2905: line 120 col 54
// CHECK-NEXT:    bc 2910: line 74 col 11
// CHECK-NEXT:    bc 2918: line 120 col 67
// CHECK-NEXT:    bc 2923: line 74 col 11
// CHECK-NEXT:    bc 2931: line 121 col 15
// CHECK-NEXT:    bc 2936: line 74 col 11
// CHECK-NEXT:    bc 2944: line 121 col 28
// CHECK-NEXT:    bc 2949: line 74 col 11
// CHECK-NEXT:    bc 2957: line 121 col 41
// CHECK-NEXT:    bc 2962: line 74 col 11
// CHECK-NEXT:    bc 2970: line 121 col 54
// CHECK-NEXT:    bc 2975: line 74 col 11
// CHECK-NEXT:    bc 2983: line 121 col 67
// CHECK-NEXT:    bc 2988: line 74 col 11
// CHECK-NEXT:    bc 2996: line 122 col 15
// CHECK-NEXT:    bc 3001: line 74 col 11
// CHECK-NEXT:    bc 3009: line 122 col 28
// CHECK-NEXT:    bc 3014: line 74 col 11
// CHECK-NEXT:    bc 3022: line 122 col 41
// CHECK-NEXT:    bc 3027: line 74 col 11
// CHECK-NEXT:    bc 3035: line 122 col 54
// CHECK-NEXT:    bc 3040: line 74 col 11
// CHECK-NEXT:    bc 3048: line 122 col 67
// CHECK-NEXT:    bc 3053: line 74 col 11
// CHECK-NEXT:    bc 3061: line 123 col 15
// CHECK-NEXT:    bc 3066: line 74 col 11
// CHECK-NEXT:    bc 3074: line 123 col 28
// CHECK-NEXT:    bc 3079: line 74 col 11
// CHECK-NEXT:    bc 3087: line 123 col 41
// CHECK-NEXT:    bc 3092: line 74 col 11
// CHECK-NEXT:    bc 3100: line 123 col 54
// CHECK-NEXT:    bc 3105: line 74 col 11
// CHECK-NEXT:    bc 3113: line 123 col 67
// CHECK-NEXT:    bc 3118: line 74 col 11
// CHECK-NEXT:    bc 3126: line 124 col 15
// CHECK-NEXT:    bc 3131: line 74 col 11
// CHECK-NEXT:    bc 3139: line 124 col 28
// CHECK-NEXT:    bc 3144: line 74 col 11
// CHECK-NEXT:    bc 3152: line 124 col 41
// CHECK-NEXT:    bc 3157: line 74 col 11
// CHECK-NEXT:    bc 3165: line 124 col 54
// CHECK-NEXT:    bc 3170: line 74 col 11
// CHECK-NEXT:    bc 3178: line 124 col 67
// CHECK-NEXT:    bc 3183: line 74 col 11
// CHECK-NEXT:    bc 3191: line 125 col 15
// CHECK-NEXT:    bc 3196: line 74 col 11
// CHECK-NEXT:    bc 3204: line 125 col 28
// CHECK-NEXT:    bc 3209: line 74 col 11
// CHECK-NEXT:    bc 3217: line 125 col 41
// CHECK-NEXT:    bc 3222: line 74 col 11
// CHECK-NEXT:    bc 3230: line 125 col 54
// CHECK-NEXT:    bc 3235: line 74 col 11
// CHECK-NEXT:    bc 3243: line 125 col 67
// CHECK-NEXT:    bc 3248: line 74 col 11
// CHECK-NEXT:    bc 3256: line 126 col 15
// CHECK-NEXT:    bc 3261: line 74 col 11
// CHECK-NEXT:    bc 3269: line 126 col 28
// CHECK-NEXT:    bc 3274: line 74 col 11
// CHECK-NEXT:    bc 3282: line 126 col 41
// CHECK-NEXT:    bc 3287: line 74 col 11
// CHECK-NEXT:    bc 3295: line 126 col 54
// CHECK-NEXT:    bc 3300: line 74 col 11
// CHECK-NEXT:    bc 3308: line 126 col 67
// CHECK-NEXT:    bc 3313: line 74 col 11
// CHECK-NEXT:    bc 3321: line 128 col 15
// CHECK-NEXT:    bc 3326: line 74 col 11
// CHECK-NEXT:    bc 3335: line 128 col 28
// CHECK-NEXT:    bc 3340: line 74 col 11
// CHECK-NEXT:    bc 3349: line 128 col 41
// CHECK-NEXT:    bc 3354: line 74 col 11
// CHECK-NEXT:  0x0785  end of debug source table
