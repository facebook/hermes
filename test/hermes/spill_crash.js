/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s
// Due to dead, unallocated instructions, this function would fail an assert.
// See T20391458.

function func1() {}
function crashMe() {
    var result = undefined;
    switch (node) {
        case 143:
            result = func1(0, result);
            break;
        case 144:
            result = func1(0, result);
            break;
        case 146:
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 147:
            result = func1(0, result);
            break;
        case 149:
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 151:
            result = func1();
            result = func1();
            result = func1(0, result);
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 152:
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 153:
            result = func1();
            result = func1();
            result = func1(0, result);
            result = func1();
            result = func1(0, result);
            break;
        case 154:
            result = func1();
            result = func1();
            result = func1(0, result);
            result = func1();
            result = func1(0, result);
            break;
        case 174:
        case 175:
            result = func1();
            break;
        case 176:
            result = func1(0, result);
            break;
        case 177:
            result = func1();
            break;
        case 178:
            result = func1();
            break;
        case 179:
            result = func1(0, result);
            break;
        case 180:
            result = func1(0, result);
            break;
        case 181:
            result = func1(0, result);
            result = func1();
            result = func1();
            break;
        case 182:
            result = func1(0, result);
            result = func1();
            result = func1();
            break;
        case 183:
            result = func1(0, result);
            break;
        case 184:
            result = func1(0, result);
            break;
        case 186:
            result = func1();
            result = func1(0, result);
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 187:
            result = func1();
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 185:
        case 188:
        case 189:
        case 190:
        case 191:
        case 197:
        case 198:
        case 203:
            result = func1(0, result);
            break;
        case 192:
        case 193:
            result = func1(0, result);
            break;
        case 194:
            result = func1(0, result);
            break;
        case 195:
            result = func1(0, result);
            break;
        case 196:
            result = func1(0, result);
            result = func1();
            break;
        case 199:
            result = func1();
            result = func1(0, result);
            break;
        case 201:
            result = func1(0, result);
            result = func1();
            break;
        case 202:
            result = func1(0, result);
            break;
        case 203:
            result = func1(0, result);
            break;
        case 205:
            result = func1(0, result);
            break;
        case 207:
            result = func1();
            break;
        case 208:
            result = func1();
            result = func1(0, result);
            break;
        case 210:
            result = func1(0, result);
            break;
        case 211:
            result = func1(0, result);
            break;
        case 212:
            result = func1(0, result);
            break;
        case 213:
        case 220:
            result = func1(0, result);
            break;
        case 214:
            result = func1(0, result);
            break;
        case 215:
        case 216:
            result = func1(0, result);
            break;
        case 219:
        case 223:
            result = func1(0, result);
            break;
        case 221:
            result = func1(0, result);
            break;
        case 222:
            result = func1(0, result);
            break;
        case 224:
            result = func1(0, result);
            break;
        case 226:
            result = func1(0, result);
            break;
        case 227:
            result = func1();
            break;
        case 228:
            result = func1();
            result = func1();
            result = func1(0, result);
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 229:
            result = func1();
            result = func1();
            result = func1(0, result);
            result = func1();
            result = func1();
            result = func1();
            break;
        case 232:
            result = func1();
            result = func1();
            result = func1(0, result);
            result = func1();
            break;
        case 233:
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 234:
            result = func1();
            break;
        case 235:
            result = func1();
            break;
        case 237:
            result = func1();
            result = func1();
            result = func1(0, result);
            break;
        case 238:
            result = func1();
            result = func1(0, result);
            break;
        case 239:
            result = func1(0, result);
            break;
        case 240:
            result = func1(0, result);
            break;
        case 241:
        case 245:
            result = func1();
            break;
        case 242:
        case 246:
            result = func1(0, result);
            break;
        case 243:
            result = func1(0, result);
            break;
        case 244:
            result = func1(0, result);
            break;
        case 248:
            result = func1(0, result);
            break;
        case 249:
            result = func1(0, result);
            break;
        case 250:
        case 251:
            result = func1(0, result);
            break;
        case 254:
            result = func1();
            break;
        case 252:
            result = func1(0, result);
            break;
    }
    return result;
}

