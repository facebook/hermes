// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_AARCH64)
#include <asmjit/a64.h>

#include <limits>
#include <stdio.h>
#include <string.h>

#include "asmjit_test_perf.h"

using namespace asmjit;

// Generates a long sequence of GP instructions.
template<typename Emitter>
static void generateGpSequenceInternal(
  Emitter& cc,
  const a64::Gp& a, const a64::Gp& b, const a64::Gp& c, const a64::Gp& d) {

  using namespace asmjit::a64;

  Gp wA = a.w();
  Gp wB = b.w();
  Gp wC = c.w();
  Gp wD = d.w();

  Gp xA = a.x();
  Gp xB = b.x();
  Gp xC = c.x();
  Gp xD = d.x();

  Mem m = ptr(xD);

  cc.mov(wA, 0);
  cc.mov(wB, 1);
  cc.mov(wC, 2);
  cc.mov(wD, 3);

  cc.adc(wA, wB, wC);
  cc.adc(xA, xB, xC);
  cc.adc(wA, wzr, wC);
  cc.adc(xA, xzr, xC);
  cc.adc(wzr, wB, wC);
  cc.adc(xzr, xB, xC);
  cc.adcs(wA, wB, wC);
  cc.adcs(xA, xB, xC);
  cc.add(wA, wB, wC);
  cc.add(xA, xB, xC);
  cc.add(wA, wB, wC, lsl(3));
  cc.add(xA, xB, xC, lsl(3));
  cc.add(wA, wzr, wC);
  cc.add(xA, xzr, xC);
  cc.add(wzr, wB, wC);
  cc.add(xzr, xB, xC);
  cc.add(wC, wD, 0, lsl(12));
  cc.add(xC, xD, 0, lsl(12));
  cc.add(wC, wD, 1024, lsl(12));
  cc.add(xC, xD, 1024, lsl(12));
  cc.add(wC, wD, 1024, lsl(12));
  cc.add(xC, xD, 1024, lsl(12));
  cc.adds(wA, wB, wC);
  cc.adds(xA, xB, xC);
  cc.adr(xA, 0);
  cc.adr(xA, 256);
  cc.adrp(xA, 4096);
  cc.and_(wA, wB, wC);
  cc.and_(xA, xB, xC);
  cc.and_(wA, wB, 1);
  cc.and_(xA, xB, 1);
  cc.and_(wA, wB, 15);
  cc.and_(xA, xB, 15);
  cc.and_(wA, wzr, wC);
  cc.and_(xA, xzr, xC);
  cc.and_(wzr, wB, wC);
  cc.and_(xzr, xB, xC);
  cc.and_(wA, wB, 0x1);
  cc.and_(xA, xB, 0x1);
  cc.and_(wA, wB, 0xf);
  cc.and_(xA, xB, 0xf);
  cc.ands(wA, wB, wC);
  cc.ands(xA, xB, xC);
  cc.ands(wA, wzr, wC);
  cc.ands(xA, xzr, xC);
  cc.ands(wzr, wB, wC);
  cc.ands(xzr, xB, xC);
  cc.ands(wA, wB, 0x1);
  cc.ands(xA, xB, 0x1);
  cc.ands(wA, wB, 0xf);
  cc.ands(xA, xB, 0xf);
  cc.asr(wA, wB, 15);
  cc.asr(xA, xB, 15);
  cc.asrv(wA, wB, wC);
  cc.asrv(xA, xB, xC);
  cc.bfc(wA, 8, 16);
  cc.bfc(xA, 8, 16);
  cc.bfi(wA, wB, 8, 16);
  cc.bfi(xA, xB, 8, 16);
  cc.bfm(wA, wB, 8, 16);
  cc.bfm(xA, xB, 8, 16);
  cc.bfxil(wA, wB, 8, 16);
  cc.bfxil(xA, xB, 8, 16);
  cc.bic(wA, wB, wC, lsl(4));
  cc.bic(xA, xB, xC, lsl(4));
  cc.bic(wA, wzr, wC);
  cc.bic(xA, xzr, xC);
  cc.bics(wA, wB, wC, lsl(4));
  cc.bics(xA, xB, xC, lsl(4));
  cc.bics(wA, wzr, wC);
  cc.bics(xA, xzr, xC);
  cc.cas(wA, wB, m);
  cc.cas(xA, xB, m);
  cc.casa(wA, wB, m);
  cc.casa(xA, xB, m);
  cc.casab(wA, wB, m);
  cc.casah(wA, wB, m);
  cc.casal(wA, wB, m);
  cc.casal(xA, xB, m);
  cc.casalb(wA, wB, m);
  cc.casalh(wA, wB, m);
  cc.casb(wA, wB, m);
  cc.cash(wA, wB, m);
  cc.casl(wA, wB, m);
  cc.casl(xA, xB, m);
  cc.caslb(wA, wB, m);
  cc.caslh(wA, wB, m);
  cc.casp(wA, wB, wC, wD, m);
  cc.casp(xA, xB, xC, xD, m);
  cc.caspa(wA, wB, wC, wD, m);
  cc.caspa(xA, xB, xC, xD, m);
  cc.caspal(wA, wB, wC, wD, m);
  cc.caspal(xA, xB, xC, xD, m);
  cc.caspl(wA, wB, wC, wD, m);
  cc.caspl(xA, xB, xC, xD, m);
  cc.ccmn(wA, wB, 3, CondCode::kEQ);
  cc.ccmn(xA, xB, 3, CondCode::kEQ);
  cc.ccmn(wA, 2, 3, CondCode::kEQ);
  cc.ccmn(xA, 2, 3, CondCode::kEQ);
  cc.ccmn(wA, wzr, 3, CondCode::kEQ);
  cc.ccmn(xA, xzr, 3, CondCode::kEQ);
  cc.ccmp(wA, wB, 3, CondCode::kEQ);
  cc.ccmp(xA, xB, 3, CondCode::kEQ);
  cc.ccmp(wA, 2, 3, CondCode::kEQ);
  cc.ccmp(xA, 2, 3, CondCode::kEQ);
  cc.ccmp(wA, wzr, 3, CondCode::kEQ);
  cc.ccmp(xA, xzr, 3, CondCode::kEQ);
  cc.cinc(wA, wB, CondCode::kEQ);
  cc.cinc(xA, xB, CondCode::kEQ);
  cc.cinc(wzr, wB, CondCode::kEQ);
  cc.cinc(wA, wzr, CondCode::kEQ);
  cc.cinc(xzr, xB, CondCode::kEQ);
  cc.cinc(xA, xzr, CondCode::kEQ);
  cc.cinv(wA, wB, CondCode::kEQ);
  cc.cinv(xA, xB, CondCode::kEQ);
  cc.cinv(wzr, wB, CondCode::kEQ);
  cc.cinv(wA, wzr, CondCode::kEQ);
  cc.cinv(xzr, xB, CondCode::kEQ);
  cc.cinv(xA, xzr, CondCode::kEQ);
  cc.cls(wA, wB);
  cc.cls(xA, xB);
  cc.cls(wA, wzr);
  cc.cls(xA, xzr);
  cc.cls(wzr, wB);
  cc.cls(xzr, xB);
  cc.clz(wA, wB);
  cc.clz(xA, xB);
  cc.clz(wA, wzr);
  cc.clz(xA, xzr);
  cc.clz(wzr, wB);
  cc.clz(xzr, xB);
  cc.cmn(wA, 33);
  cc.cmn(xA, 33);
  cc.cmn(wA, wB);
  cc.cmn(xA, xB);
  cc.cmn(wA, wB, uxtb(2));
  cc.cmn(xA, xB, uxtb(2));
  cc.cmp(wA, 33);
  cc.cmp(xA, 33);
  cc.cmp(wA, wB);
  cc.cmp(xA, xB);
  cc.cmp(wA, wB, uxtb(2));
  cc.cmp(xA, xB, uxtb(2));
  cc.crc32b(wA, wB, wC);
  cc.crc32b(wzr, wB, wC);
  cc.crc32b(wA, wzr, wC);
  cc.crc32b(wA, wB, wzr);
  cc.crc32cb(wA, wB, wC);
  cc.crc32cb(wzr, wB, wC);
  cc.crc32cb(wA, wzr, wC);
  cc.crc32cb(wA, wB, wzr);
  cc.crc32ch(wA, wB, wC);
  cc.crc32ch(wzr, wB, wC);
  cc.crc32ch(wA, wzr, wC);
  cc.crc32ch(wA, wB, wzr);
  cc.crc32cw(wA, wB, wC);
  cc.crc32cw(wzr, wB, wC);
  cc.crc32cw(wA, wzr, wC);
  cc.crc32cw(wA, wB, wzr);
  cc.crc32cx(wA, wB, xC);
  cc.crc32cx(wzr, wB, xC);
  cc.crc32cx(wA, wzr, xC);
  cc.crc32cx(wA, wB, xzr);
  cc.crc32h(wA, wB, wC);
  cc.crc32h(wzr, wB, wC);
  cc.crc32h(wA, wzr, wC);
  cc.crc32h(wA, wB, wzr);
  cc.crc32w(wA, wB, wC);
  cc.crc32w(wzr, wB, wC);
  cc.crc32w(wA, wzr, wC);
  cc.crc32w(wA, wB, wzr);
  cc.crc32x(wA, wB, xC);
  cc.crc32x(wzr, wB, xC);
  cc.crc32x(wA, wzr, xC);
  cc.crc32x(wA, wB, xzr);
  cc.csel(wA, wB, wC, CondCode::kEQ);
  cc.csel(xA, xB, xC, CondCode::kEQ);
  cc.cset(wA, CondCode::kEQ);
  cc.cset(xA, CondCode::kEQ);
  cc.cset(wA, CondCode::kEQ);
  cc.cset(xA, CondCode::kEQ);
  cc.csetm(wA, CondCode::kEQ);
  cc.csetm(xA, CondCode::kEQ);
  cc.csinc(wA, wB, wC, CondCode::kEQ);
  cc.csinc(xA, xB, xC, CondCode::kEQ);
  cc.csinv(wA, wB, wC, CondCode::kEQ);
  cc.csinv(xA, xB, xC, CondCode::kEQ);
  cc.csneg(wA, wB, wC, CondCode::kEQ);
  cc.csneg(xA, xB, xC, CondCode::kEQ);
  cc.eon(wA, wB, wC);
  cc.eon(wzr, wB, wC);
  cc.eon(wA, wzr, wC);
  cc.eon(wA, wB, wzr);
  cc.eon(wA, wB, wC, lsl(4));
  cc.eon(xA, xB, xC);
  cc.eon(xzr, xB, xC);
  cc.eon(xA, xzr, xC);
  cc.eon(xA, xB, xzr);
  cc.eon(xA, xB, xC, lsl(4));
  cc.eor(wA, wB, wC);
  cc.eor(wzr, wB, wC);
  cc.eor(wA, wzr, wC);
  cc.eor(wA, wB, wzr);
  cc.eor(xA, xB, xC);
  cc.eor(xzr, xB, xC);
  cc.eor(xA, xzr, xC);
  cc.eor(xA, xB, xzr);
  cc.eor(wA, wB, wC, lsl(4));
  cc.eor(xA, xB, xC, lsl(4));
  cc.eor(wA, wB, 0x4000);
  cc.eor(xA, xB, 0x8000);
  cc.extr(wA, wB, wC, 15);
  cc.extr(wzr, wB, wC, 15);
  cc.extr(wA, wzr, wC, 15);
  cc.extr(wA, wB, wzr, 15);
  cc.extr(xA, xB, xC, 15);
  cc.extr(xzr, xB, xC, 15);
  cc.extr(xA, xzr, xC, 15);
  cc.extr(xA, xB, xzr, 15);
  cc.ldadd(wA, wB, m);
  cc.ldadd(xA, xB, m);
  cc.ldadda(wA, wB, m);
  cc.ldadda(xA, xB, m);
  cc.ldaddab(wA, wB, m);
  cc.ldaddah(wA, wB, m);
  cc.ldaddal(wA, wB, m);
  cc.ldaddal(xA, xB, m);
  cc.ldaddalb(wA, wB, m);
  cc.ldaddalh(wA, wB, m);
  cc.ldaddb(wA, wB, m);
  cc.ldaddh(wA, wB, m);
  cc.ldaddl(wA, wB, m);
  cc.ldaddl(xA, xB, m);
  cc.ldaddlb(wA, wB, m);
  cc.ldaddlh(wA, wB, m);
  cc.ldclr(wA, wB, m);
  cc.ldclr(xA, xB, m);
  cc.ldclra(wA, wB, m);
  cc.ldclra(xA, xB, m);
  cc.ldclrab(wA, wB, m);
  cc.ldclrah(wA, wB, m);
  cc.ldclral(wA, wB, m);
  cc.ldclral(xA, xB, m);
  cc.ldclralb(wA, wB, m);
  cc.ldclralh(wA, wB, m);
  cc.ldclrb(wA, wB, m);
  cc.ldclrh(wA, wB, m);
  cc.ldclrl(wA, wB, m);
  cc.ldclrl(xA, xB, m);
  cc.ldclrlb(wA, wB, m);
  cc.ldclrlh(wA, wB, m);
  cc.ldeor(wA, wB, m);
  cc.ldeor(xA, xB, m);
  cc.ldeora(wA, wB, m);
  cc.ldeora(xA, xB, m);
  cc.ldeorab(wA, wB, m);
  cc.ldeorah(wA, wB, m);
  cc.ldeoral(wA, wB, m);
  cc.ldeoral(xA, xB, m);
  cc.ldeoralb(wA, wB, m);
  cc.ldeoralh(wA, wB, m);
  cc.ldeorb(wA, wB, m);
  cc.ldeorh(wA, wB, m);
  cc.ldeorl(wA, wB, m);
  cc.ldeorl(xA, xB, m);
  cc.ldeorlb(wA, wB, m);
  cc.ldeorlh(wA, wB, m);
  cc.ldlar(wA, m);
  cc.ldlar(xA, m);
  cc.ldlarb(wA, m);
  cc.ldlarh(wA, m);
  cc.ldnp(wA, wB, m);
  cc.ldnp(xA, xB, m);
  cc.ldp(wA, wB, m);
  cc.ldp(xA, xB, m);
  cc.ldpsw(xA, xB, m);
  cc.ldr(wA, m);
  cc.ldr(xA, m);
  cc.ldrb(wA, m);
  cc.ldrh(wA, m);
  cc.ldrsw(xA, m);
  cc.ldraa(xA, m);
  cc.ldrab(xA, m);
  cc.ldset(wA, wB, m);
  cc.ldset(xA, xB, m);
  cc.ldseta(wA, wB, m);
  cc.ldseta(xA, xB, m);
  cc.ldsetab(wA, wB, m);
  cc.ldsetah(wA, wB, m);
  cc.ldsetal(wA, wB, m);
  cc.ldsetal(xA, xB, m);
  cc.ldsetalh(wA, wB, m);
  cc.ldsetalb(wA, wB, m);
  cc.ldsetb(wA, wB, m);
  cc.ldseth(wA, wB, m);
  cc.ldsetl(wA, wB, m);
  cc.ldsetl(xA, xB, m);
  cc.ldsetlb(wA, wB, m);
  cc.ldsetlh(wA, wB, m);
  cc.ldsmax(wA, wB, m);
  cc.ldsmax(xA, xB, m);
  cc.ldsmaxa(wA, wB, m);
  cc.ldsmaxa(xA, xB, m);
  cc.ldsmaxab(wA, wB, m);
  cc.ldsmaxah(wA, wB, m);
  cc.ldsmaxal(wA, wB, m);
  cc.ldsmaxal(xA, xB, m);
  cc.ldsmaxalb(wA, wB, m);
  cc.ldsmaxalh(wA, wB, m);
  cc.ldsmaxb(wA, wB, m);
  cc.ldsmaxh(wA, wB, m);
  cc.ldsmaxl(wA, wB, m);
  cc.ldsmaxl(xA, xB, m);
  cc.ldsmaxlb(wA, wB, m);
  cc.ldsmaxlh(wA, wB, m);
  cc.ldsmin(wA, wB, m);
  cc.ldsmin(xA, xB, m);
  cc.ldsmina(wA, wB, m);
  cc.ldsmina(xA, xB, m);
  cc.ldsminab(wA, wB, m);
  cc.ldsminah(wA, wB, m);
  cc.ldsminal(wA, wB, m);
  cc.ldsminal(xA, xB, m);
  cc.ldsminalb(wA, wB, m);
  cc.ldsminalh(wA, wB, m);
  cc.ldsminb(wA, wB, m);
  cc.ldsminh(wA, wB, m);
  cc.ldsminl(wA, wB, m);
  cc.ldsminl(xA, xB, m);
  cc.ldsminlb(wA, wB, m);
  cc.ldsminlh(wA, wB, m);
  cc.ldtr(wA, m);
  cc.ldtr(xA, m);
  cc.ldtrb(wA, m);
  cc.ldtrh(wA, m);
  cc.ldtrsb(wA, m);
  cc.ldtrsh(wA, m);
  cc.ldtrsw(xA, m);
  cc.ldumax(wA, wB, m);
  cc.ldumax(xA, xB, m);
  cc.ldumaxa(wA, wB, m);
  cc.ldumaxa(xA, xB, m);
  cc.ldumaxab(wA, wB, m);
  cc.ldumaxah(wA, wB, m);
  cc.ldumaxal(wA, wB, m);
  cc.ldumaxal(xA, xB, m);
  cc.ldumaxalb(wA, wB, m);
  cc.ldumaxalh(wA, wB, m);
  cc.ldumaxb(wA, wB, m);
  cc.ldumaxh(wA, wB, m);
  cc.ldumaxl(wA, wB, m);
  cc.ldumaxl(xA, xB, m);
  cc.ldumaxlb(wA, wB, m);
  cc.ldumaxlh(wA, wB, m);
  cc.ldumin(wA, wB, m);
  cc.ldumin(xA, xB, m);
  cc.ldumina(wA, wB, m);
  cc.ldumina(xA, xB, m);
  cc.lduminab(wA, wB, m);
  cc.lduminah(wA, wB, m);
  cc.lduminal(wA, wB, m);
  cc.lduminal(xA, xB, m);
  cc.lduminalb(wA, wB, m);
  cc.lduminalh(wA, wB, m);
  cc.lduminb(wA, wB, m);
  cc.lduminh(wA, wB, m);
  cc.lduminl(wA, wB, m);
  cc.lduminl(xA, xB, m);
  cc.lduminlb(wA, wB, m);
  cc.lduminlh(wA, wB, m);
  cc.ldur(wA, m);
  cc.ldur(xA, m);
  cc.ldurb(wA, m);
  cc.ldurh(wA, m);
  cc.ldursb(wA, m);
  cc.ldursh(wA, m);
  cc.ldursw(xA, m);
  cc.ldxp(wA, wB, m);
  cc.ldxp(xA, xB, m);
  cc.ldxr(wA, m);
  cc.ldxr(xA, m);
  cc.ldxrb(wA, m);
  cc.ldxrh(wA, m);
  cc.lsl(wA, wB, wC);
  cc.lsl(xA, xB, xC);
  cc.lsl(wA, wB, 15);
  cc.lsl(xA, xB, 15);
  cc.lslv(wA, wB, wC);
  cc.lslv(xA, xB, xC);
  cc.lsr(wA, wB, wC);
  cc.lsr(xA, xB, xC);
  cc.lsr(wA, wB, 15);
  cc.lsr(xA, xB, 15);
  cc.lsrv(wA, wB, wC);
  cc.lsrv(xA, xB, xC);
  cc.madd(wA, wB, wC, wD);
  cc.madd(xA, xB, xC, xD);
  cc.mneg(wA, wB, wC);
  cc.mneg(xA, xB, xC);
  cc.mov(wA, wB);
  cc.mov(xA, xB);
  cc.mov(wA, 0);
  cc.mov(wA, 1);
  cc.mov(wA, 2);
  cc.mov(wA, 3);
  cc.mov(wA, 4);
  cc.mov(wA, 5);
  cc.mov(wA, 6);
  cc.mov(wA, 7);
  cc.mov(wA, 8);
  cc.mov(wA, 9);
  cc.mov(wA, 10);
  cc.mov(wA, 0xA234);
  cc.mov(xA, 0xA23400000000);
  cc.msub(wA, wB, wC, wD);
  cc.msub(xA, xB, xC, xD);
  cc.mul(wA, wB, wC);
  cc.mul(xA, xB, xC);
  cc.mvn(wA, wB);
  cc.mvn(xA, xB);
  cc.mvn(wA, wB, lsl(4));
  cc.mvn(xA, xB, lsl(4));
  cc.neg(wA, wB);
  cc.neg(xA, xB);
  cc.neg(wA, wB, lsl(4));
  cc.neg(xA, xB, lsl(4));
  cc.negs(wA, wB);
  cc.negs(xA, xB);
  cc.negs(wA, wB, lsl(4));
  cc.negs(xA, xB, lsl(4));
  cc.ngc(wA, wB);
  cc.ngc(xA, xB);
  cc.ngcs(wA, wB);
  cc.ngcs(xA, xB);
  cc.orn(wA, wB, wC);
  cc.orn(xA, xB, xC);
  cc.orn(wA, wB, wC, lsl(4));
  cc.orn(xA, xB, xC, lsl(4));
  cc.orr(wA, wB, wC);
  cc.orr(xA, xB, xC);
  cc.orr(wA, wB, wC, lsl(4));
  cc.orr(xA, xB, xC, lsl(4));
  cc.orr(wA, wB, 0x4000);
  cc.orr(xA, xB, 0x8000);
  cc.rbit(wA, wB);
  cc.rbit(xA, xB);
  cc.rev(wA, wB);
  cc.rev(xA, xB);
  cc.rev16(wA, wB);
  cc.rev16(xA, xB);
  cc.rev32(xA, xB);
  cc.rev64(xA, xB);
  cc.ror(wA, wB, wC);
  cc.ror(xA, xB, xC);
  cc.ror(wA, wB, 15);
  cc.ror(xA, xB, 15);
  cc.rorv(wA, wB, wC);
  cc.rorv(xA, xB, xC);
  cc.sbc(wA, wB, wC);
  cc.sbc(xA, xB, xC);
  cc.sbcs(wA, wB, wC);
  cc.sbcs(xA, xB, xC);
  cc.sbfiz(wA, wB, 5, 10);
  cc.sbfiz(xA, xB, 5, 10);
  cc.sbfm(wA, wB, 5, 10);
  cc.sbfm(xA, xB, 5, 10);
  cc.sbfx(wA, wB, 5, 10);
  cc.sbfx(xA, xB, 5, 10);
  cc.sdiv(wA, wB, wC);
  cc.sdiv(xA, xB, xC);
  cc.smaddl(xA, wB, wC, xD);
  cc.smnegl(xA, wB, wC);
  cc.smsubl(xA, wB, wC, xD);
  cc.smulh(xA, xB, xC);
  cc.smull(xA, wB, wC);
  cc.stp(wA, wB, m);
  cc.stp(xA, xB, m);
  cc.sttr(wA, m);
  cc.sttr(xA, m);
  cc.sttrb(wA, m);
  cc.sttrh(wA, m);
  cc.stur(wA, m);
  cc.stur(xA, m);
  cc.sturb(wA, m);
  cc.sturh(wA, m);
  cc.stxp(wA, wB, wC, m);
  cc.stxp(wA, xB, xC, m);
  cc.stxr(wA, wB, m);
  cc.stxr(wA, xB, m);
  cc.stxrb(wA, wB, m);
  cc.stxrh(wA, wB, m);
  cc.sub(wA, wB, wC);
  cc.sub(xA, xB, xC);
  cc.sub(wA, wB, wC, lsl(3));
  cc.sub(xA, xB, xC, lsl(3));
  cc.subg(xA, xB, 32, 11);
  cc.subp(xA, xB, xC);
  cc.subps(xA, xB, xC);
  cc.subs(wA, wB, wC);
  cc.subs(xA, xB, xC);
  cc.subs(wA, wB, wC, lsl(3));
  cc.subs(xA, xB, xC, lsl(3));
  cc.sxtb(wA, wB);
  cc.sxtb(xA, wB);
  cc.sxth(wA, wB);
  cc.sxth(xA, wB);
  cc.sxtw(xA, wB);
  cc.tst(wA, 1);
  cc.tst(xA, 1);
  cc.tst(wA, wB);
  cc.tst(xA, xB);
  cc.tst(wA, wB, lsl(4));
  cc.tst(xA, xB, lsl(4));
  cc.udiv(wA, wB, wC);
  cc.udiv(xA, xB, xC);
  cc.ubfiz(wA, wB, 5, 10);
  cc.ubfiz(xA, xB, 5, 10);
  cc.ubfm(wA, wB, 5, 10);
  cc.ubfm(xA, xB, 5, 10);
  cc.ubfx(wA, wB, 5, 10);
  cc.ubfx(xA, xB, 5, 10);
  cc.umaddl(xA, wB, wC, xD);
  cc.umnegl(xA, wB, wC);
  cc.umsubl(xA, wB, wC, xD);
  cc.umulh(xA, xB, xC);
  cc.umull(xA, wB, wC);
  cc.uxtb(wA, wB);
  cc.uxth(wA, wB);
}

static void generateGpSequence(BaseEmitter& emitter, bool emitPrologEpilog) {
  if (emitter.isAssembler()) {
    a64::Assembler& cc = *emitter.as<a64::Assembler>();

    a64::Gp a = a64::x0;
    a64::Gp b = a64::x1;
    a64::Gp c = a64::x2;
    a64::Gp d = a64::x3;

    if (emitPrologEpilog) {
      FuncDetail func;
      func.init(FuncSignature::build<void, void*, const void*, size_t>(), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.addDirtyRegs(a, b, c, d);
      frame.finalize();

      cc.emitProlog(frame);
      generateGpSequenceInternal(cc, a, b, c, d);
      cc.emitEpilog(frame);
    }
    else {
      generateGpSequenceInternal(cc, a, b, c, d);
    }
  }
#ifndef ASMJIT_NO_BUILDER
  else if (emitter.isBuilder()) {
    a64::Builder& cc = *emitter.as<a64::Builder>();

    a64::Gp a = a64::x0;
    a64::Gp b = a64::x1;
    a64::Gp c = a64::x2;
    a64::Gp d = a64::x3;

    if (emitPrologEpilog) {
      FuncDetail func;
      func.init(FuncSignature::build<void, void*, const void*, size_t>(), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.addDirtyRegs(a, b, c, d);
      frame.finalize();

      cc.emitProlog(frame);
      generateGpSequenceInternal(cc, a, b, c, d);
      cc.emitEpilog(frame);
    }
    else {
      generateGpSequenceInternal(cc, a, b, c, d);
    }
  }
#endif
#ifndef ASMJIT_NO_COMPILER
  else if (emitter.isCompiler()) {
    a64::Compiler& cc = *emitter.as<a64::Compiler>();

    a64::Gp a = cc.newIntPtr("a");
    a64::Gp b = cc.newIntPtr("b");
    a64::Gp c = cc.newIntPtr("c");
    a64::Gp d = cc.newIntPtr("d");

    cc.addFunc(FuncSignature::build<void>());
    generateGpSequenceInternal(cc, a, b, c, d);
    cc.endFunc();
  }
#endif
}

template<typename EmitterFn>
static void benchmarkA64Function(Arch arch, uint32_t numIterations, const char* description, const EmitterFn& emitterFn) noexcept {
  CodeHolder code;
  printf("%s:\n", description);

  uint32_t instCount = 0;

#ifndef ASMJIT_NO_BUILDER
  instCount = asmjit_perf_utils::calculateInstructionCount<a64::Builder>(code, arch, [&](a64::Builder& cc) {
    emitterFn(cc, false);
  });
#endif

  asmjit_perf_utils::bench<a64::Assembler>(code, arch, numIterations, "[raw]", instCount, [&](a64::Assembler& cc) {
    emitterFn(cc, false);
  });

  asmjit_perf_utils::bench<a64::Assembler>(code, arch, numIterations, "[validated]", instCount, [&](a64::Assembler& cc) {
    cc.addDiagnosticOptions(DiagnosticOptions::kValidateAssembler);
    emitterFn(cc, false);
  });

  asmjit_perf_utils::bench<a64::Assembler>(code, arch, numIterations, "[prolog/epilog]", instCount, [&](a64::Assembler& cc) {
    cc.addDiagnosticOptions(DiagnosticOptions::kValidateAssembler);
    emitterFn(cc, true);
  });

#ifndef ASMJIT_NO_BUILDER
  asmjit_perf_utils::bench<a64::Builder>(code, arch, numIterations, "[no-asm]", instCount, [&](a64::Builder& cc) {
    emitterFn(cc, false);
  });

  asmjit_perf_utils::bench<a64::Builder>(code, arch, numIterations, "[finalized]", instCount, [&](a64::Builder& cc) {
    emitterFn(cc, false);
    cc.finalize();
  });

  asmjit_perf_utils::bench<a64::Builder>(code, arch, numIterations, "[prolog/epilog]", instCount, [&](a64::Builder& cc) {
    emitterFn(cc, true);
    cc.finalize();
  });
#endif

#ifndef ASMJIT_NO_COMPILER
  asmjit_perf_utils::bench<a64::Compiler>(code, arch, numIterations, "[no-asm]", instCount, [&](a64::Compiler& cc) {
    emitterFn(cc, true);
  });

  asmjit_perf_utils::bench<a64::Compiler>(code, arch, numIterations, "[finalized]", instCount, [&](a64::Compiler& cc) {
    emitterFn(cc, true);
    cc.finalize();
  });
#endif

  printf("\n");
}

void benchmarkA64Emitters(uint32_t numIterations) {
  static const char description[] = "GpSequence (Sequence of GP instructions - reg/mem)";
  benchmarkA64Function(Arch::kAArch64, numIterations, description, [](BaseEmitter& emitter, bool emitPrologEpilog) {
    generateGpSequence(emitter, emitPrologEpilog);
  });
}

#endif // !ASMJIT_NO_AARCH64
