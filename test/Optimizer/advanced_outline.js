// RUN: %hermesc -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=1 -outline-min-params=0 -outline-max-params=5 -dump-ir %s | %FileCheck --match-full-lines %s

// Notes:
// - Each test uses a unique name test{Name} to avoid conflicts.
// - Negative tests (expecting no outlining) are named negTest{Name}.

// =============================================================================
// Outline 3 functions that are structurally the same but use different variable
// names. Do not outline the 4th one, which has a different literal value.
// =============================================================================

//CHECK-LABEL:function testOutlineSome_xyz(x) : undefined
//CHECK:  %0 = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}} : undefined, undefined : undefined, %x
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
function testOutlineSome_xyz(x) {
  var y = x + 2;
  var z = y * y;
  print(x - z + 1);
  a(b(c(d(e(x)))));
}

//CHECK-LABEL:function testOutlineSome_ijk(i) : undefined
//CHECK:  %0 = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}} : undefined, undefined : undefined, %i
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
function testOutlineSome_ijk(i) {
  var j = i + 2;
  var k = j * j;
  print(i - k + 1);
  a(b(c(d(e(i)))));
}

//CHECK-LABEL:function testOutlineSome_qrs(q) : undefined
//CHECK:  %0 = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}} : undefined, undefined : undefined, %q
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
function testOutlineSome_qrs(q) {
  var r = q + 2;
  var s = r * r;
  print(q - s + 1);
  a(b(c(d(e(q)))));
}

//CHECK-LABEL:function testOutlineSome_differentLiteral(x) : undefined
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function testOutlineSome_differentLiteral(x) {
  var y = x + 2;
  var z = y * y;
  print(x - z + 5); // 5 instead of 1
  a(b(c(d(e(x)))));
}

// =============================================================================
// DO NOT outline functions that are structurally different.
// =============================================================================

//CHECK-LABEL:function negTestStructure_1() : string|number
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestStructure_1() {
  var x = foo();
  var y = bar();
  return x + y;
}

//CHECK-LABEL:function negTestStructure_2() : string|number
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestStructure_2() {
  var x = foo();
  var y = bar();
  return y + x;
}

// =============================================================================
// Outline 2 functions in which one has a value that escapes (becoming the
// outlined function return value) and the other doesn't.
// =============================================================================

//CHECK-LABEL:function testCompatibleEscape_noEscape(x) : undefined
//CHECK:  %0 = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}}
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
function testCompatibleEscape_noEscape(x) {
  var y = x.t2.t2.t2.t2.t2.t2.t2.t2;
  var z = y.t2.t2.t2.t2.t2.t2.t2.t2;
}

//CHECK-LABEL:function testCompatibleEscape_oneEscape(x)
//CHECK:  %0 = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}}
//CHECK-NEXT:  %1 = ReturnInst %0
function testCompatibleEscape_oneEscape(x) {
  var y = x.t2.t2.t2.t2.t2.t2.t2.t2;
  var z = y.t2.t2.t2.t2.t2.t2.t2.t2;
  return y;
}

// =============================================================================
// DO NOT outline when candidates have conflicting values that escape.
// =============================================================================

//CHECK-LABEL:function negTestConflictingEscape_oneEscapeAt8(x)
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestConflictingEscape_oneEscapeAt8(x) {
  var y = x.t3.t3.t3.t3.t3.t3.t3.t3;
  var z = y.t3.t3.t3.t3.t3.t3.t3.t3;
  return y;
}

//CHECK-LABEL:function negTestConflictingEscape_oneEscapeAt9(x)
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestConflictingEscape_oneEscapeAt9(x) {
  var y = x.t3.t3.t3.t3.t3.t3.t3.t3.t3;
  var z = y.t3.t3.t3.t3.t3.t3.t3;
  return y;
}

// =============================================================================
// DO NOT outline when there are two values that escape (and shortening it to
// just one escape would render the block too short to be worth outlining).
// =============================================================================

//CHECK-LABEL:function negTestTwoEscapes_1(x) : string|number
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestTwoEscapes_1(x) {
  var y = x.t4.t4.t4.t4.t4.t4;
  var z = x.t4.t4.t4.t4.t4.t4;
  var w = x.t4.t4.t4.t4.t4.t4;
  thing1(); // prevent it from outlining everything (y + z becoming 1 escape)
  return y + z;
}

//CHECK-LABEL:function negTestTwoEscapes_2(x) : string|number
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestTwoEscapes_2(x) {
  var y = x.t4.t4.t4.t4.t4.t4;
  var z = x.t4.t4.t4.t4.t4.t4;
  var w = x.t4.t4.t4.t4.t4.t4;
  thing2(); // prevent it from outlining everything (y + z becoming 1 escape)
  return y + z;
}

// =============================================================================
// DO NOT outline when the instructions have the same ValueKind but different
// sub-kind (in this case, BinaryOperatorInst::OpKind).
// =============================================================================

//CHECK-LABEL:function negTestOpKind_plus(x) : string|number
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestOpKind_plus(x) {
  return x + x + x + x + x + x + x + x + x + x + x + x + x + x + x + x + x;
}

//CHECK-LABEL:function negTestOpKind_minus(x) : number
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestOpKind_minus(x) {
  return x - x - x - x - x - x - x - x - x - x - x - x - x - x - x - x - x;
}

// =============================================================================
// DO NOT outline multiple basic blocks.
// =============================================================================

//CHECK-LABEL:function negTestMultipleblocks_1(x) : undefined
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestMultipleblocks_1(x) {
  for (;;) {
    if (x) x++;
    if (x) x++;
    if (x) x++;
    if (x) x++;
  }
}

//CHECK-LABEL:function negTestMultipleblocks_2(x) : undefined
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestMultipleblocks_2(x) {
  for (;;) {
    if (x) x++;
    if (x) x++;
    if (x) x++;
    if (x) x++;
  }
}

// =============================================================================
// DO NOT outline instructions that touch stack variables.
// =============================================================================

//CHECK-LABEL:function negTestStackVariable_1(x)
//CHECK: %0 = AllocStackInst $a
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestStackVariable_1(x) {
  var a = x;
  try {
    a = a.t7.t7.t7.t7.t7.t7.t7.t7;
    a = a.t7.t7.t7.t7.t7.t7.t7.t7;
  } catch (e) {
    return a;
  }
}

//CHECK-LABEL:function negTestStackVariable_2(x)
//CHECK: %0 = AllocStackInst $a
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestStackVariable_2(x) {
  var a = x;
  try {
    a = a.t7.t7.t7.t7.t7.t7.t7.t7;
    a = a.t7.t7.t7.t7.t7.t7.t7.t7;
  } catch (e) {
    return a;
  }
}

// =============================================================================
// DO NOT outline instructions that touch frame variables.
// =============================================================================

//CHECK-LABEL:function negTestFrameVariable_1(x, f) : undefined
//CHECK-NEXT: frame = [x]
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestFrameVariable_1(x, f) {
  x = x.t8.t8.t8.t8.t8.t8.t8.t8;
  f(function() {
    x++;
  });
  x = x.t8.t8.t8.t8.t8.t8.t8.t8;
}

//CHECK-LABEL:function negTestFrameVariable_2(x, f) : undefined
//CHECK-NEXT: frame = [x]
//CHECK-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}
function negTestFrameVariable_2(x, f) {
  x = x.t8.t8.t8.t8.t8.t8.t8.t8;
  f(function() {
    x++;
  });
  x = x.t8.t8.t8.t8.t8.t8.t8.t8;
}

// =============================================================================
// Outline in cases where multiple parameters have to be passed into the
// outlined function.
// =============================================================================

//CHECK-LABEL:function testMultipleParams_1() : undefined
//CHECK:  {{.*}} = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}} : undefined, undefined : undefined, {{.*}}, {{.*}}, {{.*}}
function testMultipleParams_1() {
  var x = thing1();
  var y = thing1();
  var z = thing1();
  x.t9.t9.t9.t9.t9.t9;
  y.t9.t9.t9.t9.t9.t9;
  z.t9.t9.t9.t9.t9.t9;
}

//CHECK-LABEL:function testMultipleParams_2() : undefined
//CHECK:  {{.*}} = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}} : undefined, undefined : undefined, {{.*}}, {{.*}}, {{.*}}
function testMultipleParams_2() {
  var x = thing2();
  var y = thing2();
  var z = thing2();
  x.t9.t9.t9.t9.t9.t9;
  y.t9.t9.t9.t9.t9.t9;
  z.t9.t9.t9.t9.t9.t9;
}

//CHECK-LABEL:function testMultipleParams_3() : undefined
//CHECK:  {{.*}} = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}} : undefined, undefined : undefined, {{.*}}, {{.*}}, {{.*}}
function testMultipleParams_3() {
  var x = thing3();
  var y = thing3();
  var z = thing3();
  x.t9.t9.t9.t9.t9.t9;
  y.t9.t9.t9.t9.t9.t9;
  z.t9.t9.t9.t9.t9.t9;
}

// =============================================================================
// Outline repeated code within a single function.
// =============================================================================

//CHECK-LABEL:function testSingleFunction(x) : undefined
//CHECK:  {{.*}} = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}}
//CHECK:  {{.*}} = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}}
function testSingleFunction(x) {
  x.t10.t10.t10.t10.t10.t10.t10.t10;
  x.t10.t10.t10.t10.t10.t10.t10.t10;
  barrier();
  x.t10.t10.t10.t10.t10.t10.t10.t10;
  x.t10.t10.t10.t10.t10.t10.t10.t10;
}

// End with a label for the first outlined function definition, so that checks
// above don't match in the outlined functions.

//CHECK-LABEL:function OUTLINED_FUNCTION{{.*}}
