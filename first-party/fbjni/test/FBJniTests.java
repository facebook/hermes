/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.facebook.jni;

import static org.fest.assertions.api.Assertions.assertThat;
import static org.mockito.Mockito.verify;

import com.facebook.jni.annotations.DoNotStrip;
import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import org.fest.assertions.api.Fail;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.runners.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class FBJniTests extends BaseFBJniTests {
  class CustomException extends Throwable {
    int mGetMessageCalls = 0;

    @Override
    public String getMessage() {
      return "getMessages: " + (++mGetMessageCalls);
    }
  }

  public interface Callbacks {
    void voidFoo();

    boolean booleanFoo();

    byte byteFoo();

    char charFoo();

    short shortFoo();

    int intFoo();

    long longFoo();

    float floatFoo();

    double doubleFoo();

    Object objectFoo();

    String stringFoo();
  }

  public static class TestThing {
    int foo;
  }

  @Mock private static Callbacks mCallbacksMock;

  private int mIntFieldTest;
  private String mStringFieldTest;
  private TestThing mReferenceFieldTest;
  private static int sIntFieldTest;
  private static String sStringFieldTest;
  private static TestThing sReferenceFieldTest;

  @DoNotStrip // Resolved from fbjni_tests::TestFieldAccess
  int bar(double d) {
    return 42;
  }

  // Test case for nonvirtual function
  public boolean nonVirtualMethod(boolean s) {
    return s;
  }

  private static void verifyAllCallbacksCalled(Callbacks mock) {
    verify(mock).voidFoo();
    verify(mock).booleanFoo();
    verify(mock).byteFoo();
    verify(mock).charFoo();
    verify(mock).shortFoo();
    verify(mock).intFoo();
    verify(mock).longFoo();
    verify(mock).floatFoo();
    verify(mock).doubleFoo();
    verify(mock).objectFoo();
    verify(mock).stringFoo();
  }

  // Instead of mocking, lets call non-static functions and verify them.
  public static void voidFooStatic() {
    mCallbacksMock.voidFoo();
  }

  public static boolean booleanFooStatic() {
    return mCallbacksMock.booleanFoo();
  }

  public static byte byteFooStatic() {
    return mCallbacksMock.byteFoo();
  }

  public static char charFooStatic(char c, int s) {
    return mCallbacksMock.charFoo();
  }

  public static short shortFooStatic(short s, short t) {
    return mCallbacksMock.shortFoo();
  }

  public static int intFooStatic(int s) {
    return mCallbacksMock.intFoo();
  }

  public static long longFooStatic() {
    return mCallbacksMock.longFoo();
  }

  public static float floatFooStatic() {
    return mCallbacksMock.floatFoo();
  }

  public static double doubleFooStatic() {
    return mCallbacksMock.doubleFoo();
  }

  public static Object objectFooStatic() {
    return mCallbacksMock.objectFoo();
  }

  public static String stringFooStatic() {
    return mCallbacksMock.stringFoo();
  }

  @Test
  public void resolveClass() throws ClassNotFoundException {
    assertThat(nativeTestClassResolution("java/lang/Object")).isTrue();
  }

  // Some versions of Android throw ClassNotFoundException while others throw NoClassDefFoundError.
  // Flatten that to always be ClassNotFoundException.
  private static void wrapClassLoadingErrors(Callable<?> code) throws Exception {
    try {
      code.call();
    } catch (NoClassDefFoundError ex) {
      throw new ClassNotFoundException("chained NoClassDefFoundError", ex);
    }
  }

  @Test(expected = ClassNotFoundException.class)
  public void failingToResolveClass() throws Exception {
    wrapClassLoadingErrors(
        new Callable<Boolean>() {
          @Override
          public Boolean call() throws Exception {
            return nativeTestClassResolution("ThisClassDoesNotExist");
          }
        });
  }

  private native boolean nativeTestClassResolution(String className) throws ClassNotFoundException;

  @Test
  public void lazyClassResolution() throws ClassNotFoundException {
    assertThat(nativeTestLazyClassResolution("java/lang/Object")).isTrue();
  }

  @Test(expected = ClassNotFoundException.class)
  public void failedLazyClassResolution() throws Exception {
    wrapClassLoadingErrors(
        new Callable<Boolean>() {
          @Override
          public Boolean call() throws Exception {
            return nativeTestLazyClassResolution("ThisClassDoesNotExist");
          }
        });
  }

  private native boolean nativeTestLazyClassResolution(String className)
      throws ClassNotFoundException;

  @Test
  public void instanceCreation() {
    assertThat(nativeCreateInstanceOf("java/lang/String"))
        .isInstanceOf(String.class)
        .isEqualTo("java/lang/String");
  }

  private native Object nativeCreateInstanceOf(String className);

  @Test
  public void typeDescriptors() {
    assertThat(nativeTestTypeDescriptors()).isTrue();
  }

  private native boolean nativeTestTypeDescriptors();

  @Test
  public void resolveVirtualMethod() throws ClassNotFoundException, NoSuchMethodException {
    assertThat(nativeTestVirtualMethodResolution_I("java/lang/Object", "hashCode")).isTrue();
  }

  @Test
  public void resolveVirtualMethodWithArray() throws ClassNotFoundException, NoSuchMethodException {
    assertThat(nativeTestVirtualMethodResolution_arrB("java/lang/String", "getBytes")).isTrue();
  }

  @Test
  public void resolveVirtualMethodWithObjectArray()
      throws ClassNotFoundException, NoSuchMethodException {
    assertThat(nativeTestVirtualMethodResolution_S_arrS("java/lang/String", "split")).isTrue();
  }

  @Test
  public void resolveVirtualMethodWithObjectArrayArray()
      throws ClassNotFoundException, NoSuchMethodException {
    assertThat(
            nativeTestVirtualMethodResolution_arrarrS(
                "com/facebook/jni/FBJniTests", "returnMultidimensionalObjectArray"))
        .isTrue();
  }

  public static String[][] returnMultidimensionalObjectArray() {
    return null;
  }

  @Test
  public void resolveVirtualMethodWithPrimitiveArrayArray()
      throws ClassNotFoundException, NoSuchMethodException {
    assertThat(
            nativeTestVirtualMethodResolution_arrarrI(
                "com/facebook/jni/FBJniTests", "returnMultidimensionalPrimitiveArray"))
        .isTrue();
  }

  public static int[][] returnMultidimensionalPrimitiveArray() {
    return null;
  }

  @Test(expected = NoSuchMethodError.class)
  public void failingToResolveVirtualMethod() throws ClassNotFoundException, NoSuchMethodError {
    nativeTestVirtualMethodResolution_I("java/lang/Object", "ThisMethodDoesNotExist");
  }

  private native boolean nativeTestVirtualMethodResolution_I(String className, String methodName)
      throws ClassNotFoundException, NoSuchMethodError;

  private native boolean nativeTestVirtualMethodResolution_arrB(String className, String methodName)
      throws ClassNotFoundException, NoSuchMethodError;

  private native boolean nativeTestVirtualMethodResolution_S_arrS(
      String className, String methodName) throws ClassNotFoundException, NoSuchMethodError;

  private native boolean nativeTestVirtualMethodResolution_arrarrS(
      String className, String methodName) throws ClassNotFoundException, NoSuchMethodError;

  private native boolean nativeTestVirtualMethodResolution_arrarrI(
      String className, String methodName) throws ClassNotFoundException, NoSuchMethodError;

  @Test
  public void lazyMethodResolution() throws ClassNotFoundException, NoSuchMethodError {
    assertThat(nativeTestLazyVirtualMethodResolution_I("java/lang/Object", "hashCode")).isTrue();
  }

  @Test(expected = NoSuchMethodError.class)
  public void failedLazyMethodResolution() throws ClassNotFoundException, NoSuchMethodError {
    nativeTestLazyVirtualMethodResolution_I("java/lang/Object", "ThisMethodDoesNotExist");
  }

  private native boolean nativeTestLazyVirtualMethodResolution_I(
      String className, String methodName);

  @Test
  public void callbacksUsingJMethod() {
    nativeTestJMethodCallbacks(mCallbacksMock);
    verifyAllCallbacksCalled(mCallbacksMock);
  }

  private native void nativeTestJMethodCallbacks(Callbacks callbacks);

  @Test
  public void callbacksUsingJStaticMethod() {
    nativeTestJStaticMethodCallbacks();
    verifyAllCallbacksCalled(mCallbacksMock);
  }

  private native void nativeTestJStaticMethodCallbacks();

  @Test
  public void isAssignableFrom() {
    assertThat(nativeTestIsAssignableFrom(String.class, String.class)).isTrue();
    assertThat(nativeTestIsAssignableFrom(String.class, Object.class)).isFalse();
    assertThat(nativeTestIsAssignableFrom(Object.class, String.class)).isTrue();
    assertThat(nativeTestIsAssignableFrom(ArrayList.class, Iterable.class)).isFalse();
    assertThat(nativeTestIsAssignableFrom(Iterable.class, ArrayList.class)).isTrue();
  }

  private native boolean nativeTestIsAssignableFrom(Class cls1, Class cls2);

  @Test
  public void isInstanceOf() {
    assertThat(nativeTestIsInstanceOf("", String.class)).isTrue();
    assertThat(nativeTestIsInstanceOf("", Object.class)).isTrue();
    assertThat(nativeTestIsInstanceOf(new Object(), String.class)).isFalse();
    assertThat(nativeTestIsInstanceOf(new ArrayList(), Iterable.class)).isTrue();
    assertThat(nativeTestIsInstanceOf(null, Iterable.class)).isTrue();
  }

  private native boolean nativeTestIsInstanceOf(Object object, Class cls);

  @Test
  public void isSameObject() {
    Object anObject = new Object();
    Object anotherObject = new Object();
    assertThat(nativeTestIsSameObject(anObject, anObject)).isTrue();
    assertThat(nativeTestIsSameObject(anObject, anotherObject)).isFalse();
    assertThat(nativeTestIsSameObject(null, anObject)).isFalse();
    assertThat(nativeTestIsSameObject(anObject, null)).isFalse();
    assertThat(nativeTestIsSameObject(null, null)).isTrue();
  }

  private native boolean nativeTestIsSameObject(Object a, Object b);

  @Test
  public void testGetSuperClass() {
    Class testClass = String.class;
    Class superClass = Object.class;
    Class notSuperClass = Integer.class;

    assertThat(nativeTestGetSuperclass(testClass, superClass)).isTrue();
    assertThat(nativeTestGetSuperclass(testClass, notSuperClass)).isFalse();
  }

  private native boolean nativeTestGetSuperclass(Class testClass, Class superOfTest);

  @Test
  public void testWeakRefs() {
    assertThat(nativeTestWeakRefs()).isTrue();
  }

  private native boolean nativeTestWeakRefs();

  @Test
  public void testAliasRefs() {
    assertThat(nativeTestAlias()).isTrue();
  }

  private native boolean nativeTestAlias();

  @Test
  public void testAliasRefConversions() {
    assertThat(nativeTestAliasRefConversions()).isTrue();
  }

  private native boolean nativeTestAliasRefConversions();

  @Test
  public void testNullJString() {
    assertThat(nativeTestNullJString()).isTrue();
  }

  private native boolean nativeTestNullJString();

  @Test
  public void testSwap() {
    assertThat(nativeTestSwap(new Object())).isTrue();
  }

  private native boolean nativeTestSwap(Object other);

  @Test
  public void testEqualOperator() {
    assertThat(nativeTestEqualOperator(new Object())).isTrue();
  }

  private native boolean nativeTestEqualOperator(Object other);

  @Test
  public void testRelaseAlias() {
    assertThat(nativeTestReleaseAlias()).isTrue();
  }

  private native boolean nativeTestReleaseAlias();

  @Test
  public void testLockingWeakReferences() {
    assertThat(nativeTestLockingWeakReferences()).isTrue();
  }

  private native boolean nativeTestLockingWeakReferences();

  @Test
  public void testCreatingReferences() {
    assertThat(nativeTestCreatingReferences()).isTrue();
  }

  private native boolean nativeTestCreatingReferences();

  @Test
  public void testAssignmentAndCopyConstructors() {
    assertThat(nativeTestAssignmentAndCopyConstructors()).isTrue();
  }

  private native boolean nativeTestAssignmentAndCopyConstructors();

  @Test
  public void testAssignmentAndCopyCrossTypes() {
    assertThat(nativeTestAssignmentAndCopyCrossTypes()).isTrue();
  }

  private native boolean nativeTestAssignmentAndCopyCrossTypes();

  @Test
  public void testNullReferences() {
    assertThat(nativeTestNullReferences()).isTrue();
  }

  private native boolean nativeTestNullReferences();

  @Test
  public void testAutoAliasRefReturningVoid() {
    nativeTestAutoAliasRefReturningVoid();
  }

  private native void nativeTestAutoAliasRefReturningVoid();

  @Test
  public void testFieldAccess() {
    mIntFieldTest = 17;
    assertThat(nativeTestFieldAccess("mIntFieldTest", mIntFieldTest, 42)).isTrue();
    assertThat(mIntFieldTest).isEqualTo(42);
  }

  private native boolean nativeTestFieldAccess(String name, int oldVal, int newVal);

  @Test
  public void testStringFieldAccess() {
    mStringFieldTest = "initial";
    assertThat(nativeTestStringFieldAccess("mStringFieldTest", mStringFieldTest, "final")).isTrue();
    assertThat(mStringFieldTest).isEqualTo("final");
  }

  private native boolean nativeTestStringFieldAccess(String name, String oldVal, String newVal);

  @Test
  public void testReferenceFieldAccess() {
    for (boolean useWrapper : new boolean[] {false, true}) {
      mReferenceFieldTest = new TestThing();
      TestThing newthing = new TestThing();

      assertThat(
              nativeTestReferenceFieldAccess(
                  "mReferenceFieldTest", mReferenceFieldTest, newthing, useWrapper))
          .isTrue();
      assertThat(mReferenceFieldTest).isEqualTo(newthing);
    }
  }

  private native boolean nativeTestReferenceFieldAccess(
      String name, Object oldVal, Object newVal, boolean useWrapper);

  @Test
  public void testStaticFieldAccess() {
    sIntFieldTest = 17;
    assertThat(nativeTestStaticFieldAccess("sIntFieldTest", sIntFieldTest, 42)).isTrue();
    assertThat(sIntFieldTest).isEqualTo(42);
  }

  private native boolean nativeTestStaticFieldAccess(String name, int oldVal, int newVal);

  @Test
  public void testStaticStringFieldAccess() {
    sStringFieldTest = "initial";
    assertThat(nativeTestStaticStringFieldAccess("sStringFieldTest", sStringFieldTest, "final"))
        .isTrue();
    assertThat(sStringFieldTest).isEqualTo("final");
  }

  private native boolean nativeTestStaticStringFieldAccess(String name, String oVal, String nVal);

  @Test
  public void testStaticReferenceFieldAccess() {
    for (boolean useWrapper : new boolean[] {false, true}) {
      sReferenceFieldTest = new TestThing();
      TestThing newthing = new TestThing();

      assertThat(
              nativeTestStaticReferenceFieldAccess(
                  "sReferenceFieldTest", sReferenceFieldTest, newthing, useWrapper))
          .isTrue();
      assertThat(sReferenceFieldTest).isEqualTo(newthing);
    }
  }

  private native boolean nativeTestStaticReferenceFieldAccess(
      String name, Object oldVal, Object newVal, boolean useWrapper);

  @Test
  public void testNonVirtualMethod() {
    assertThat(nativeTestNonVirtualMethod(true)).isTrue();
  }

  private native boolean nativeTestNonVirtualMethod(boolean s);

  @Test
  public void testArrayCreation() {
    String[] expectedStrings = {"one", "two", "three"};
    String[] joinedStrings =
        nativeTestArrayCreation(expectedStrings[0], expectedStrings[1], expectedStrings[2]);
    assertThat(joinedStrings).isEqualTo(expectedStrings);
  }

  private native String[] nativeTestArrayCreation(String s0, String s1, String s2);

  @Test
  public void testMultidimensionalObjectArray() {
    String[] strings = {"one", "two", "three"};
    String[][] expectedStrings = {{"one", "two"}, {"three"}};
    String[][] joinedStrings =
        nativeTestMultidimensionalObjectArray(strings[0], strings[1], strings[2]);
    assertThat(joinedStrings).isEqualTo(expectedStrings);
  }

  private native String[][] nativeTestMultidimensionalObjectArray(String s0, String s1, String s2);

  @Test
  public void testMultidimensionalPrimitiveArray() {
    int[] nums = {1, 2, 3};
    int[][] expectedNums = {{1, 2}, {3}};
    int[][] gotNums = nativeTestMultidimensionalPrimitiveArray(nums[0], nums[1], nums[2]);
    assertThat(gotNums).isEqualTo(expectedNums);
  }

  private native int[][] nativeTestMultidimensionalPrimitiveArray(int i0, int i1, int i2);

  private String[] mCapturedStringArray = null;

  @DoNotStrip
  String captureStringArray(String[] input) {
    mCapturedStringArray = input;
    return "Stub";
  }

  @Test
  public void testBuildStringArray() throws Exception {
    String[] input = {"Four", "score", "and", "seven", "beers", "ago"};
    nativeTestBuildStringArray(input);
    assertThat(mCapturedStringArray).isEqualTo(input);
  }

  private native String nativeTestBuildStringArray(String... input);

  public Object methodResolutionWithCxxTypes(String t, long val) {
    if (!"test".equals(t) || val != 3) throw new RuntimeException();
    return null;
  }

  public void methodResolutionWithCxxTypesVoid(String t, long val) {
    if (!"test".equals(t) || val != 3) throw new RuntimeException();
  }

  public int methodResolutionWithCxxTypesInt(String t, long val) {
    if (!"test".equals(t) || val != 3) throw new RuntimeException();
    return 0;
  }

  public static Object methodResolutionWithCxxTypesStatic(String t, long val) {
    if (!"test".equals(t) || val != 3) throw new RuntimeException();
    return null;
  }

  public static void methodResolutionWithCxxTypesVoidStatic(String t, long val) {
    if (!"test".equals(t) || val != 3) throw new RuntimeException();
  }

  public static int methodResolutionWithCxxTypesIntStatic(String t, long val) {
    if (!"test".equals(t) || val != 3) throw new RuntimeException();
    return 0;
  }

  @Test
  public void testMethodResolutionWithCxxTypes() {
    testMethodResolutionWithCxxTypesNative("methodResolutionWithCxxTypes", "test", 3);
  }

  private native void testMethodResolutionWithCxxTypesNative(
      String callbackName, String str, long val);

  @Test(expected = CustomException.class)
  public void testHandleJavaCustomException() {
    testHandleJavaCustomExceptionNative();
  }

  private native void testHandleJavaCustomExceptionNative();

  @Test
  public void testHandleNullExceptionMessage() {
    testHandleNullExceptionMessageNative();
  }

  private native void testHandleNullExceptionMessageNative();

  @Test
  public void testHandleNestedException() {
    try {
      nativeTestHandleNestedException();
    } catch (Throwable e) {
      assertThat(e).isInstanceOf(ArrayIndexOutOfBoundsException.class);
      e = e.getCause();
      assertThat(e).isInstanceOf(RuntimeException.class);
      e = e.getCause();
      assertThat(e).isInstanceOf(CustomException.class).hasNoCause();
    }
  }

  private native void nativeTestHandleNestedException();

  @Test(expected = CppException.class)
  public void testHandleNoRttiException() {
    nativeTestHandleNoRttiException();
  }

  private native void nativeTestHandleNoRttiException();

  @Test
  public void testCopyConstructor() {
    assertThat(nativeTestCopyConstructor())
        .isEqualTo("com.facebook.jni.FBJniTests$CustomException: getMessages: 1");
  }

  private native String nativeTestCopyConstructor();

  @Test
  public void testMoveConstructorWithEmptyWhat() {
    assertThat(nativeTestMoveConstructorWithEmptyWhat())
        .isEqualTo("com.facebook.jni.FBJniTests$CustomException: getMessages: 1");
  }

  private native String nativeTestMoveConstructorWithEmptyWhat();

  @Test
  public void testMoveConstructorWithPopulatedWhat() {
    assertThat(nativeTestMoveConstructorWithPopulatedWhat())
        .isEqualTo("com.facebook.jni.FBJniTests$CustomException: getMessages: 1");
  }

  private native String nativeTestMoveConstructorWithPopulatedWhat();

  @DoNotStrip // Used in native code.
  protected void customExceptionThrower() throws CustomException {
    throw new CustomException();
  }

  @DoNotStrip // Used in native code.
  protected void nullMessageThrower() throws NullPointerException {
    // just like Preconditions.checkNotNull() does
    throw new NullPointerException();
  }

  @Test
  public void testHandleCppRuntimeError() {
    String message = "Sample runtime error.";
    thrown.expect(RuntimeException.class);
    thrown.expectMessage(message);
    nativeTestHandleCppRuntimeError(message);
  }

  private native void nativeTestHandleCppRuntimeError(String message);

  @Test(expected = IOException.class)
  public void testHandleCppIOBaseFailure() {
    nativeTestHandleCppIOBaseFailure();
  }

  private native void nativeTestHandleCppIOBaseFailure();

  @Test(expected = CppSystemErrorException.class)
  public void testHandleCppSystemError() {
    nativeTestHandleCppSystemError();
  }

  private native void nativeTestHandleCppSystemError();

  @Test(expected = RuntimeException.class)
  public void testInterDsoExceptionHandlingA() {
    nativeTestInterDsoExceptionHandlingA();
  }

  private native void nativeTestInterDsoExceptionHandlingA();

  @Test
  public void testInterDsoExceptionHandlingB() {
    assertThat(nativeTestInterDsoExceptionHandlingB()).isTrue();
  }

  private native boolean nativeTestInterDsoExceptionHandlingB();

  @Test
  public void testHandleNonStdExceptionThrow() {
    try {
      nativeTestHandleNonStdExceptionThrow();
      Fail.failBecauseExceptionWasNotThrown(UnknownCppException.class);
    } catch (UnknownCppException ex) {
      if (System.getProperty("os.name").startsWith("Windows")) {
        // Unknown exception types not supported on Windows.
        assertThat(ex.getMessage()).isEqualTo("Unknown");
        return;
      }
      // the actual string is implementation-defined and mangled, but in practice,
      // it has the name of the C++ type in it somewhere.
      assertThat(ex.getMessage()).startsWith("Unknown: ").contains("NonStdException");
    }
  }

  private native void nativeTestHandleNonStdExceptionThrow();

  @Test(expected = UnknownCppException.class)
  public void testHandleCppCharPointerThrow() {
    nativeTestHandleCppCharPointerThrow();
  }

  private native void nativeTestHandleCppCharPointerThrow();

  @Test(expected = IllegalArgumentException.class)
  public void testThrowJavaExceptionByName() {
    nativeTestThrowJavaExceptionByName();
  }

  private native void nativeTestThrowJavaExceptionByName();

  @Test(expected = UnknownCppException.class)
  public void testHandleCppIntThrow() {
    nativeTestHandleCppIntThrow();
  }

  private native void nativeTestHandleCppIntThrow();

  @Test
  public void testJThread() {
    assertThat(nativeTestJThread()).isEqualTo(1);
  }

  private native int nativeTestJThread();

  @Test
  public void testThreadScopeGuard() {
    assertThat(nativeTestThreadScopeGuard(17)).isEqualTo(42);
  }

  private native int nativeTestThreadScopeGuard(double input);

  @Test
  public void testNestedThreadScopeGuard() {
    assertThat(nativeTestNestedThreadScopeGuard(17)).isEqualTo(42);
  }

  private native int nativeTestNestedThreadScopeGuard(double input);

  @Test
  public void testClassLoadInWorker() {
    assertThat(nativeTestClassLoadInWorker()).isEqualTo(1);
  }

  private native int nativeTestClassLoadInWorker();

  @Test
  public void testClassLoadWorkerFastPath() {
    assertThat(nativeTestClassLoadWorkerFastPath()).isEqualTo(3);
  }

  private native int nativeTestClassLoadWorkerFastPath();

  @Test
  public void testToString() {
    assertThat(nativeTestToString()).isTrue();
  }

  private native boolean nativeTestToString();

  // Casting alias_ref

  @Test
  public void testCorrectStaticCastAliasRef() {
    // Static cast can't fail at run time.  If the object isn't actually
    // of that type, we just get undefined behaviour, which we can't
    // check for.  So we only do a positive test.
    assertThat(nativeStaticCastAliasRefToString("hello")).isTrue();
  }

  @Test
  public void testNullStaticCastAliasRef() {
    assertThat(nativeStaticCastAliasRefToString(null)).isTrue();
  }

  private native boolean nativeStaticCastAliasRefToString(Object a);

  @Test
  public void testDynamicCastAliasRefToSame() {
    assertThat(nativeDynamicCastAliasRefToThrowable(new Throwable())).isTrue();
  }

  public void testDynamicCastAliasRefToBase() {
    assertThat(nativeDynamicCastAliasRefToThrowable(new Exception())).isTrue();
  }

  @Test(expected = ClassCastException.class)
  public void testDynamicCastAliasRefToDerived() {
    nativeDynamicCastAliasRefToThrowable(new Object());
  }

  @Test(expected = ClassCastException.class)
  public void testDynamicCastAliasRefToUnrelated() {
    nativeDynamicCastAliasRefToThrowable(new Integer(23));
  }

  @Test
  public void testNullDynamicCastAliasRef() {
    assertThat(nativeDynamicCastAliasRefToThrowable(null)).isTrue();
  }

  private native boolean nativeDynamicCastAliasRefToThrowable(Object a);

  // Casting local_ref

  @Test
  public void testCorrectStaticCastLocalRef() {
    // Static cast can't fail at run time.  If the object isn't actually
    // of that type, we just get undefined behaviour, which we can't
    // check for.  So we only do a positive test.
    assertThat(nativeStaticCastLocalRefToString("hello")).isTrue();
  }

  @Test
  public void testNullStaticCastLocalRef() {
    assertThat(nativeStaticCastLocalRefToString(null)).isTrue();
  }

  private native boolean nativeStaticCastLocalRefToString(Object a);

  @Test
  public void testCorrectDynamicCastLocalRef() {
    assertThat(nativeDynamicCastLocalRefToString("hello")).isTrue();
  }

  @Test(expected = ClassCastException.class)
  public void testIncorrectDynamicCastLocalRef() {
    nativeDynamicCastLocalRefToString(new Integer(23));
  }

  @Test
  public void testNullDynamicCastLocalRef() {
    assertThat(nativeDynamicCastLocalRefToString(null)).isTrue();
  }

  private native boolean nativeDynamicCastLocalRefToString(Object a);

  // Casting global_ref

  @Test
  public void testCorrectStaticCastGlobalRef() {
    // Static cast can't fail at run time.  If the object isn't actually
    // of that type, we just get undefined behaviour, which we can't
    // check for.  So we only do a positive test.
    assertThat(nativeStaticCastGlobalRefToString("hello")).isTrue();
  }

  @Test
  public void testNullStaticCastGlobalRef() {
    assertThat(nativeStaticCastGlobalRefToString(null)).isTrue();
  }

  private native boolean nativeStaticCastGlobalRefToString(Object a);

  @Test
  public void testCorrectDynamicCastGlobalRef() {
    assertThat(nativeDynamicCastGlobalRefToString("hello")).isTrue();
  }

  @Test(expected = ClassCastException.class)
  public void testIncorrectDynamicCastGlobalRef() {
    nativeDynamicCastGlobalRefToString(new Integer(23));
  }

  @Test
  public void testNullDynamicCastGlobalRef() {
    assertThat(nativeDynamicCastGlobalRefToString(null)).isTrue();
  }

  private native boolean nativeDynamicCastGlobalRefToString(Object a);

  @Test
  public void testCriticalNativeMethodBindsAndCanBeInvoked() {
    assertThat(nativeCriticalNativeMethodBindsAndCanBeInvoked(12, 3.45f)).isTrue();
  }

  private static native boolean nativeCriticalNativeMethodBindsAndCanBeInvoked(int a, float b);
}
