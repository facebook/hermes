// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#if !defined(ASMJIT_NO_COMPILER) && !defined(ASMJIT_NO_AARCH64)

#include <asmjit/a64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./asmjit_test_compiler.h"

using namespace asmjit;

// a64::Compiler - A64TestCase
// ===========================

class A64TestCase : public TestCase {
public:
  A64TestCase(const char* name = nullptr)
    : TestCase(name, Arch::kAArch64) {}

  virtual void compile(BaseCompiler& cc) override {
    compile(static_cast<a64::Compiler&>(cc));
  }

  virtual void compile(a64::Compiler& cc) = 0;
};

// a64::Compiler - A64Test_GpArgs
// ==============================

class A64Test_GpArgs : public A64TestCase {
public:
  uint32_t _argCount;
  bool _preserveFP;

  A64Test_GpArgs(uint32_t argCount, bool preserveFP)
    : _argCount(argCount),
      _preserveFP(preserveFP) {
    _name.assignFormat("GpArgs {NumArgs=%u PreserveFP=%c}", argCount, preserveFP ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
      app.add(new A64Test_GpArgs(i, true));
      app.add(new A64Test_GpArgs(i, false));
    }
  }

  virtual void compile(a64::Compiler& cc) {
    uint32_t i;
    uint32_t argCount = _argCount;

    FuncSignature signature;
    signature.setRetT<int>();
    for (i = 0; i < argCount; i++)
      signature.addArgT<int>();

    FuncNode* funcNode = cc.addFunc(signature);
    if (_preserveFP)
      funcNode->frame().setPreservedFP();

    a64::Gp sum;

    if (argCount) {
      for (i = 0; i < argCount; i++) {
        a64::Gp iReg = cc.newInt32("i%u", i);
        funcNode->setArg(i, iReg);

        if (i == 0)
          sum = iReg;
        else
          cc.add(sum, sum, iReg);
      }
    }
    else {
      sum = cc.newInt32("i");
      cc.mov(sum, 0);
    }

    cc.ret(sum);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef unsigned int U;

    typedef U (*Func0)();
    typedef U (*Func1)(U);
    typedef U (*Func2)(U, U);
    typedef U (*Func3)(U, U, U);
    typedef U (*Func4)(U, U, U, U);
    typedef U (*Func5)(U, U, U, U, U);
    typedef U (*Func6)(U, U, U, U, U, U);
    typedef U (*Func7)(U, U, U, U, U, U, U);
    typedef U (*Func8)(U, U, U, U, U, U, U, U);
    typedef U (*Func9)(U, U, U, U, U, U, U, U, U);
    typedef U (*Func10)(U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func11)(U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func12)(U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func13)(U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func14)(U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func15)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func16)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    switch (_argCount) {
      case 0:
        resultRet = ptr_as_func<Func0>(_func)();
        expectRet = 0;
        break;
      case 1:
        resultRet = ptr_as_func<Func1>(_func)(1);
        expectRet = 1;
        break;
      case 2:
        resultRet = ptr_as_func<Func2>(_func)(1, 2);
        expectRet = 1 + 2;
        break;
      case 3:
        resultRet = ptr_as_func<Func3>(_func)(1, 2, 3);
        expectRet = 1 + 2 + 3;
        break;
      case 4:
        resultRet = ptr_as_func<Func4>(_func)(1, 2, 3, 4);
        expectRet = 1 + 2 + 3 + 4;
        break;
      case 5:
        resultRet = ptr_as_func<Func5>(_func)(1, 2, 3, 4, 5);
        expectRet = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        resultRet = ptr_as_func<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6;
        break;
      case 7:
        resultRet = ptr_as_func<Func7>(_func)(1, 2, 3, 4, 5, 6, 7);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7;
        break;
      case 8:
        resultRet = ptr_as_func<Func8>(_func)(1, 2, 3, 4, 5, 6, 7, 8);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
        break;
      case 9:
        resultRet = ptr_as_func<Func9>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
        break;
      case 10:
        resultRet = ptr_as_func<Func10>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
        break;
      case 11:
        resultRet = ptr_as_func<Func11>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
        break;
      case 12:
        resultRet = ptr_as_func<Func12>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12;
        break;
      case 13:
        resultRet = ptr_as_func<Func13>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13;
        break;
      case 14:
        resultRet = ptr_as_func<Func14>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14;
        break;
      case 15:
        resultRet = ptr_as_func<Func15>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15;
        break;
      case 16:
        resultRet = ptr_as_func<Func16>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
        break;
    }

    result.assignFormat("ret={%u, %u}", resultRet >> 28, resultRet & 0x0FFFFFFFu);
    expect.assignFormat("ret={%u, %u}", expectRet >> 28, expectRet & 0x0FFFFFFFu);

    return result == expect;
  }
};

// a64::Compiler - A64Test_Simd1
// =============================

class A64Test_Simd1 : public A64TestCase {
public:
  A64Test_Simd1()
    : A64TestCase("Simd1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Simd1());
  }

  virtual void compile(a64::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, const void*, const void*>());

    a64::Gp dst = cc.newUIntPtr("dst");
    a64::Gp src1 = cc.newUIntPtr("src1");
    a64::Gp src2 = cc.newUIntPtr("src2");

    funcNode->setArg(0, dst);
    funcNode->setArg(1, src1);
    funcNode->setArg(2, src2);

    a64::Vec v1 = cc.newVecQ("vec1");
    a64::Vec v2 = cc.newVecQ("vec2");
    a64::Vec v3 = cc.newVecQ("vec3");

    cc.ldr(v2, a64::ptr(src1));
    cc.ldr(v3, a64::ptr(src2));
    cc.add(v1.b16(), v2.b16(), v3.b16());
    cc.str(v1, a64::ptr(dst));

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, const void*, const void*);

    uint32_t dst[4];
    uint32_t aSrc[4] = { 0 , 1 , 2 , 255 };
    uint32_t bSrc[4] = { 99, 17, 33, 1   };

    // NOTE: It's a byte-add, so uint8_t(255+1) == 0.
    uint32_t ref[4] = { 99, 18, 35, 0 };

    ptr_as_func<Func>(_func)(dst, aSrc, bSrc);

    result.assignFormat("ret={%u, %u, %u, %u}", dst[0], dst[1], dst[2], dst[3]);
    expect.assignFormat("ret={%u, %u, %u, %u}", ref[0], ref[1], ref[2], ref[3]);

    return result == expect;
  }
};

// a64::Compiler - A64Test_ManyRegs
// ================================

class A64Test_ManyRegs : public A64TestCase {
public:
  uint32_t _regCount;

  A64Test_ManyRegs(uint32_t n)
    : A64TestCase(),
      _regCount(n) {
    _name.assignFormat("GpRegs {NumRegs=%u}", n);
  }

  static void add(TestApp& app) {
    for (uint32_t i = 2; i < 64; i++)
      app.add(new A64Test_ManyRegs(i));
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    a64::Gp* regs = static_cast<a64::Gp*>(malloc(_regCount * sizeof(a64::Gp)));

    for (uint32_t i = 0; i < _regCount; i++) {
      regs[i] = cc.newUInt32("reg%u", i);
      cc.mov(regs[i], i + 1);
    }

    a64::Gp sum = cc.newUInt32("sum");
    cc.mov(sum, 0);

    for (uint32_t i = 0; i < _regCount; i++) {
      cc.add(sum, sum, regs[i]);
    }

    cc.ret(sum);
    cc.endFunc();

    free(regs);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assignFormat("ret={%d}", func());
    expect.assignFormat("ret={%d}", calcSum());

    return result == expect;
  }

  uint32_t calcSum() const {
    return (_regCount | 1) * ((_regCount + 1) / 2);
  }
};

// a64::Compiler - A64Test_Adr
// ===========================

class A64Test_Adr : public A64TestCase {
public:
  A64Test_Adr()
    : A64TestCase("Adr") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Adr());
  }

  virtual void compile(a64::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    a64::Gp addr = cc.newIntPtr("addr");
    a64::Gp val = cc.newIntPtr("val");

    Label L_Table = cc.newLabel();

    cc.adr(addr, L_Table);
    cc.ldrsw(val, a64::ptr(addr, 8));
    cc.ret(val);
    cc.endFunc();

    cc.bind(L_Table);
    cc.embedInt32(1);
    cc.embedInt32(2);
    cc.embedInt32(3);
    cc.embedInt32(4);
    cc.embedInt32(5);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assignFormat("ret={%d}", func());
    expect.assignFormat("ret={%d}", 3);

    return result == expect;
  }
};

// a64::Compiler - A64Test_Branch1
// ===============================

class A64Test_Branch1 : public A64TestCase {
public:
  A64Test_Branch1()
    : A64TestCase("Branch1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Branch1());
  }

  virtual void compile(a64::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, size_t>());

    a64::Gp p = cc.newIntPtr("p");
    a64::Gp count = cc.newIntPtr("count");
    a64::Gp i = cc.newIntPtr("i");
    Label L = cc.newLabel();

    funcNode->setArg(0, p);
    funcNode->setArg(1, count);

    cc.mov(i, 0);

    cc.bind(L);
    cc.strb(i.w(), a64::ptr(p, i));
    cc.add(i, i, 1);
    cc.cmp(i, count);
    cc.b_ne(L);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void* p, size_t n);
    Func func = ptr_as_func<Func>(_func);

    uint8_t array[16];
    func(array, 16);

    expect.assign("ret={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}");

    result.assign("ret={");
    for (size_t i = 0; i < 16; i++) {
      if (i)
        result.append(", ");
      result.appendFormat("%d", int(array[i]));
    }
    result.append("}");

    return result == expect;
  }
};

// a64::Compiler - A64Test_Invoke1
// ===============================

class A64Test_Invoke1 : public A64TestCase {
public:
  A64Test_Invoke1()
    : A64TestCase("Invoke1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke1());
  }

  virtual void compile(a64::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint32_t, uint32_t, uint32_t>());

    a64::Gp x = cc.newUInt32("x");
    a64::Gp y = cc.newUInt32("y");
    a64::Gp r = cc.newUInt32("r");
    a64::Gp fn = cc.newUIntPtr("fn");

    funcNode->setArg(0, x);
    funcNode->setArg(1, y);

    cc.mov(fn, (uint64_t)calledFunc);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignature::build<uint32_t, uint32_t, uint32_t>());
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint32_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t x = 49;
    uint32_t y = 7;

    result.assignFormat("ret={%u}", func(x, y));
    expect.assignFormat("ret={%u}", x - y);

    return result == expect;
  }

  static uint32_t calledFunc(uint32_t x, uint32_t y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke2
// ===============================

class A64Test_Invoke2 : public A64TestCase {
public:
  A64Test_Invoke2()
    : A64TestCase("Invoke2") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke2());
  }

  virtual void compile(a64::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, double, double>());

    a64::Vec x = cc.newVecD("x");
    a64::Vec y = cc.newVecD("y");
    a64::Vec r = cc.newVecD("r");
    a64::Gp fn = cc.newUIntPtr("fn");

    funcNode->setArg(0, x);
    funcNode->setArg(1, y);
    cc.mov(fn, (uint64_t)calledFunc);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignature::build<double, double, double>());
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double x = 49;
    double y = 7;

    result.assignFormat("ret={%f}", func(x, y));
    expect.assignFormat("ret={%f}", calledFunc(x, y));

    return result == expect;
  }

  static double calledFunc(double x, double y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke3
// ===============================

class A64Test_Invoke3 : public A64TestCase {
public:
  A64Test_Invoke3()
    : A64TestCase("Invoke3") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke3());
  }

  virtual void compile(a64::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, double, double>());

    a64::Vec x = cc.newVecD("x");
    a64::Vec y = cc.newVecD("y");
    a64::Vec r = cc.newVecD("r");
    a64::Gp fn = cc.newUIntPtr("fn");

    funcNode->setArg(0, x);
    funcNode->setArg(1, y);
    cc.mov(fn, (uint64_t)calledFunc);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, fn, FuncSignature::build<double, double, double>());
    invokeNode->setArg(0, y);
    invokeNode->setArg(1, x);
    invokeNode->setRet(0, r);

    cc.ret(r);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double x = 49;
    double y = 7;

    result.assignFormat("ret={%f}", func(x, y));
    expect.assignFormat("ret={%f}", calledFunc(y, x));

    return result == expect;
  }

  static double calledFunc(double x, double y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_JumpTable
// =================================

class A64Test_JumpTable : public A64TestCase {
public:
  bool _annotated;

  A64Test_JumpTable(bool annotated)
    : A64TestCase("A64Test_JumpTable"),
      _annotated(annotated) {
    _name.assignFormat("JumpTable {%s}", annotated ? "Annotated" : "Unknown Target");
  }

  enum Operator {
    kOperatorAdd = 0,
    kOperatorSub = 1,
    kOperatorMul = 2,
    kOperatorDiv = 3
  };

  static void add(TestApp& app) {
    app.add(new A64Test_JumpTable(false));
    app.add(new A64Test_JumpTable(true));
  }

  virtual void compile(a64::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<float, float, float, uint32_t>());

    a64::Vec a = cc.newVecS("a");
    a64::Vec b = cc.newVecS("b");
    a64::Gp op = cc.newUInt32("op");

    a64::Gp target = cc.newIntPtr("target");
    a64::Gp offset = cc.newIntPtr("offset");

    Label L_End = cc.newLabel();

    Label L_Table = cc.newLabel();
    Label L_Add = cc.newLabel();
    Label L_Sub = cc.newLabel();
    Label L_Mul = cc.newLabel();
    Label L_Div = cc.newLabel();

    funcNode->setArg(0, a);
    funcNode->setArg(1, b);
    funcNode->setArg(2, op);

    cc.adr(target, L_Table);
    cc.ldrsw(offset, a64::ptr(target, op, a64::sxtw(2)));
    cc.add(target, target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      annotation->addLabel(L_Add);
      annotation->addLabel(L_Sub);
      annotation->addLabel(L_Mul);
      annotation->addLabel(L_Div);
      cc.br(target, annotation);
    }
    else {
      cc.br(target);
    }

    cc.bind(L_Add);
    cc.fadd(a, a, b);
    cc.b(L_End);

    cc.bind(L_Sub);
    cc.fsub(a, a, b);
    cc.b(L_End);

    cc.bind(L_Mul);
    cc.fmul(a, a, b);
    cc.b(L_End);

    cc.bind(L_Div);
    cc.fdiv(a, a, b);

    cc.bind(L_End);
    cc.ret(a);
    cc.endFunc();

    cc.bind(L_Table);
    cc.embedLabelDelta(L_Add, L_Table, 4);
    cc.embedLabelDelta(L_Sub, L_Table, 4);
    cc.embedLabelDelta(L_Mul, L_Table, 4);
    cc.embedLabelDelta(L_Div, L_Table, 4);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef float (*Func)(float, float, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    float dst[4];
    float ref[4];

    dst[0] = func(33.0f, 14.0f, kOperatorAdd);
    dst[1] = func(33.0f, 14.0f, kOperatorSub);
    dst[2] = func(10.0f, 6.0f, kOperatorMul);
    dst[3] = func(80.0f, 8.0f, kOperatorDiv);

    ref[0] = 47.0f;
    ref[1] = 19.0f;
    ref[2] = 60.0f;
    ref[3] = 10.0f;

    result.assignFormat("ret={%f, %f, %f, %f}", dst[0], dst[1], dst[2], dst[3]);
    expect.assignFormat("ret={%f, %f, %f, %f}", ref[0], ref[1], ref[2], ref[3]);

    return result == expect;
  }
};

// a64::Compiler - Export
// ======================

void compiler_add_a64_tests(TestApp& app) {
  app.addT<A64Test_GpArgs>();
  app.addT<A64Test_ManyRegs>();
  app.addT<A64Test_Simd1>();
  app.addT<A64Test_Adr>();
  app.addT<A64Test_Branch1>();
  app.addT<A64Test_Invoke1>();
  app.addT<A64Test_Invoke2>();
  app.addT<A64Test_Invoke3>();
  app.addT<A64Test_JumpTable>();
}

#endif // !ASMJIT_NO_COMPILER && !ASMJIT_NO_AARCH64
