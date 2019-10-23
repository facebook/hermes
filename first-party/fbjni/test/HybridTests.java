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

import com.facebook.jni.annotations.DoNotStrip;
import org.junit.Test;

public class HybridTests extends BaseFBJniTests {
  static class TestHybridClass {
    // Hybrid classes must include a member which manages the C++ object.  It
    // will be initialized from C++.  It must be declared exactly with this
    // type and name, so JNI can find it, and initialized once in the ctor.
    // The annotation is necessary to keep proguard from renaming it, or else JNI
    // won't be able to find it.
    @DoNotStrip private final HybridData mHybridData;

    // This is the method which creates the C++ instance and initializes
    // mHybridData.  Conventionally, it should be named initHybrid, and invoked
    // from the constructor.  This must be called only once.  If the C++
    // instance is referenced before this is called, a NullPointerException
    // will be thrown.
    private native HybridData initHybrid(int i, String s, boolean b);

    // You can have more than one, which may be useful if the ctor is
    // overloaded.  This will call the default C++ ctor.
    private native HybridData initHybrid();

    // Implements factory-style initialization.  You shouldn't usually
    // need both styles in one class.  Here we do it for testing and
    // demo purposes.
    private native HybridData initHybrid(String s, int i, boolean b);

    // Java ctor must invoke initHybrid().  This just passes arguments through,
    // but the ctor can do whatever work it wants, as long as it calls
    // initHybrid() before any native methods.
    public TestHybridClass(int i, String s, boolean b) {
      mHybridData = initHybrid(i, s, b);
    }

    // This behaves the same as the ctor above, I just wanted a different
    // signature to demonstrate factory-style initialization.
    public TestHybridClass(String s, int i, boolean b) {
      mHybridData = initHybrid(s, i, b);
    }

    // This is the simplest case.  Even if everything is default, initHybrid()
    // must still be called.
    public TestHybridClass() {
      mHybridData = initHybrid();
    }

    // Java ctor used by C++ newObjectCxxArgs.  Note this is private.
    private TestHybridClass(HybridData hd) {
      mHybridData = hd;
    }

    public void doneUsingIt() {
      mHybridData.resetNative();
    }

    // Some C++ methods.
    public native void setBoth(int i, String s);

    public native int getInt();

    public native String getString();

    public native String getCharString();

    public native boolean copy1(TestHybridClass other);

    public native boolean copy2(TestHybridClass other);

    public native void oops();

    public native void setGlobal(String s);

    public native String getGlobal1();

    public native String getGlobal2();

    public static native TestHybridClass makeWithTwo();

    public static native TestHybridClass makeWithThree();

    public static native void autoconvertMany();
  }

  @Test
  public void testHybridClass() {
    TestHybridClass thc1 = new TestHybridClass();
    assertThat(thc1.getInt()).isEqualTo(0);
    assertThat(thc1.getString()).isEqualTo("");

    thc1.setBoth(1, "one");
    assertThat(thc1.getInt()).isEqualTo(1);
    assertThat(thc1.getString()).isEqualTo("one");

    TestHybridClass thc2 = TestHybridClass.makeWithTwo();
    assertThat(thc2.getInt()).isEqualTo(2);
    assertThat(thc2.getString()).isEqualTo("two");

    thc2.doneUsingIt();

    thrown.expect(NullPointerException.class);
    thc2.getInt();
  }

  @Test
  public void testHybridAutoconversion() {
    TestHybridClass thc3 = TestHybridClass.makeWithThree();
    assertThat(thc3.copy1(new TestHybridClass(3, "three", false))).isTrue();
    assertThat(thc3.getInt()).isEqualTo(3);
    assertThat(thc3.getString()).isEqualTo("three");

    TestHybridClass thc4 = new TestHybridClass();
    thc4.copy1(new TestHybridClass("four", 4, false));
    assertThat(thc4.getInt()).isEqualTo(4);
    assertThat(thc4.getString()).isEqualTo("four");
    assertThat(thc4.getCharString()).isEqualTo("four");

    TestHybridClass thc5 = new TestHybridClass();
    assertThat(thc5.copy2(new TestHybridClass(5, "five", false))).isTrue();
    assertThat(thc5.getInt()).isEqualTo(5);
    assertThat(thc5.getString()).isEqualTo("five");
  }

  @Test
  public void testReturnGlobalRef() {
    TestHybridClass thc = new TestHybridClass();
    thc.setGlobal("global_ref");
    assertThat(thc.getGlobal1()).isEqualTo("global_ref");
    assertThat(thc.getGlobal2()).isEqualTo("global_ref");
  }

  @Test
  public void testLocalLeak() {
    TestHybridClass.autoconvertMany();
  }

  @Test
  public void testExceptionMapping() {
    TestHybridClass thc1 = new TestHybridClass();
    thrown.expect(ArrayStoreException.class);
    thc1.oops();
  }

  abstract static class AbstractTestHybrid {
    @DoNotStrip private final HybridData mHybridData;

    private int mAbstractNum;

    protected AbstractTestHybrid(HybridData hybridData, int an) {
      mHybridData = hybridData;
      mAbstractNum = an;
    }

    public int abstractNum() {
      return mAbstractNum;
    }

    public native int nativeNum();

    public abstract int concreteNum();

    public abstract int sum();
  }

  static class ConcreteTestHybrid extends AbstractTestHybrid {
    public ConcreteTestHybrid(int an, int nn, int cn) {
      super(initHybrid(nn, cn), an);
    }

    private static native HybridData initHybrid(int nn, int cn);

    // overrides can be native
    @Override
    public native int concreteNum();

    // overrides can be java
    @Override
    public int sum() {
      return nativeNum() + abstractNum() + concreteNum();
    }
  }

  @Test
  public void testHybridInheritance() {
    AbstractTestHybrid ath = new ConcreteTestHybrid(1, 2, 3);
    assertThat(ath.abstractNum()).isEqualTo(1);
    assertThat(ath.nativeNum()).isEqualTo(2);
    assertThat(ath.concreteNum()).isEqualTo(3);
    assertThat(ath.sum()).isEqualTo(6);
  }

  public static native boolean cxxTestInheritance(AbstractTestHybrid ath);

  public static native AbstractTestHybrid makeAbstractHybrid();

  @Test
  public void testHybridCxx() {
    AbstractTestHybrid ath = new ConcreteTestHybrid(4, 5, 6);
    assertThat(cxxTestInheritance(ath)).isTrue();

    AbstractTestHybrid ath2 = makeAbstractHybrid();
    assertThat(ath2 instanceof ConcreteTestHybrid).isTrue();
    assertThat(ath2.abstractNum()).isEqualTo(7);
    assertThat(ath2.nativeNum()).isEqualTo(8);
    assertThat(ath2.concreteNum()).isEqualTo(9);
    assertThat(ath2.sum()).isEqualTo(24);
  }

  static class Base {}

  static class Derived extends Base {
    @DoNotStrip private final HybridData mHybridData;

    private Derived(HybridData hybridData) {
      mHybridData = hybridData;
    }
  }

  public static native boolean cxxTestDerivedJavaClass();

  @Test
  public void testDerivedJavaClassCxx() {
    assertThat(cxxTestDerivedJavaClass()).isTrue();
  }

  static class TestHybridClassBase extends HybridClassBase {
    protected native void initHybrid();

    private native void initHybrid(int i);

    protected TestHybridClassBase() {
      // No initHybrid() here!
      // Otherwise factory construction will set native pointer twice and process will crash.
    }

    public TestHybridClassBase(int i) {
      initHybrid(i);
    }

    // Some C++ methods.
    public native void setInt(int i);

    public native int getInt();

    public static native TestHybridClassBase makeWithThree();
  }

  static class TestHybridClassBaseDefaultCtor extends TestHybridClassBase {
    public TestHybridClassBaseDefaultCtor() {
      initHybrid();
    }
  }

  @Test
  public void testHybridBaseDefaultCtor() {
    TestHybridClassBaseDefaultCtor base = new TestHybridClassBaseDefaultCtor();
    assertThat(base.getInt()).isZero();

    base.setInt(58);
    assertThat(base.getInt()).isEqualTo(58);
  }

  @Test
  public void testHybridBaseConstructorArgs() {
    TestHybridClassBase base = new TestHybridClassBase(42);
    assertThat(base.getInt()).isEqualTo(42);
  }

  @Test
  public void testHybridBaseFactoryConstruction() {
    TestHybridClassBase base = TestHybridClassBase.makeWithThree();
    assertThat(base.getInt()).isEqualTo(3);
  }

  static class Destroyable {
    @DoNotStrip private final HybridData mHybridData;

    private Destroyable(HybridData hybridData) {
      mHybridData = hybridData;
    }
  }

  public static native boolean cxxTestHybridDestruction();

  @Test
  public void testHybridDestuction() {
    assertThat(cxxTestHybridDestruction()).isTrue();
  }
}
