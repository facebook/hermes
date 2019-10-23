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

#include <fbjni/fbjni.h>
#include <mutex>
#include <condition_variable>

using namespace facebook::jni;

class TestException : public std::runtime_error {
public:
  TestException() : std::runtime_error("fail") {}
};

// The C++ half of a hybrid class must extend HybridClass like this.
// HybridClass is default constructible.
class TestHybridClass : public facebook::jni::HybridClass<TestHybridClass> {
public:
  static constexpr const char* const kJavaDescriptor =
    "Lcom/facebook/jni/HybridTests$TestHybridClass;";

  TestHybridClass() {}

  TestHybridClass(int i, std::string s, bool b)
    : i_(i)
    , s_(std::move(s))
    , b_(b) {}

  // The C++ implementation of initHybrid must be a static C++ method,
  // since it has no C++ instance (it creates it).  But it's a normal
  // java method, because it gets a java this passed in (which is
  // often unused).  It must call makeCxxInstance and return its
  // result.
  static jhybriddata initHybrid(alias_ref<jhybridobject>, jint i, jstring s, jboolean b) {
    // The arguments will be forwarded to the ctor, and the result
    // will be saved in mHybridPointer in the java object.
    return makeCxxInstance(i, wrap_alias(s)->toStdString(), b).release();
  }
  static local_ref<jhybriddata> initHybrid2(alias_ref<jhybridobject>) {
    return makeCxxInstance();
  }
  // It's also ok to make initHybrid work like a factory, which
  // eliminates the need to define a ctor and reduces boilerplate
  // code.  This is a member, so fields can be initialized directly.
  static local_ref<jhybriddata> initHybrid3(alias_ref<jhybridobject>,
                                            std::string s,
                                            int i,
                                            bool b) {
    auto cxxPart = std::unique_ptr<TestHybridClass>(new TestHybridClass);
    cxxPart->s_ = std::move(s);
    cxxPart->i_ = i;
    cxxPart->b_ = b;
    return makeHybridData(std::move(cxxPart));
  }

  jint getInt() {
    return i_;
  }

  std::string getString() {
    return s_;
  }

  const char* getCharString() {
    return s_.c_str();
  }

  void setBoth(int i, std::string s) {
    i_ = i;
    s_ = s;
  }

  bool copy1(alias_ref<jhybridobject> other) {
    i_ = cthis(other)->i_;
    s_ = cthis(other)->s_;
    return true;
  }

  // This returns a boolean to test more code paths, not for any
  // functional reason.
  jboolean copy2(TestHybridClass* other) {
    i_ = other->i_;
    s_ = other->s_;
    return true;
  }

  void oops() {
    throw TestException();
  }

  void setGlobal(alias_ref<jstring> str) {
    global_ = make_global(str);
  }

  global_ref<jstring> getGlobal1() {
    return global_;
  }

  const global_ref<jstring>& getGlobal2() {
    return global_;
  }

  static jhybridobject makeWithTwo(alias_ref<jclass> jthis) {
    // The args are passed to java, with autoconversion.
    return newObjectJavaArgs(2, "two", true).release();
  }

  static local_ref<jhybridobject> makeWithThree(alias_ref<jclass> jthis) {
    // The args are passed to C++ initialization directly.
   return newObjectCxxArgs(3, "three", true);
  }

  static void mapException(const std::exception& ex) {
    if (dynamic_cast<const TestException*>(&ex) != 0) {
      throwNewJavaException("java/lang/ArrayStoreException", "");
    }
  }

  static void autoconvertMany(alias_ref<jclass>) {
    // Autoconversion to jstring creates a local ref.  Make sure is is
    // released properly.  The default local table size is 512.
    for (int i = 0; i < 1000; ++i) {
      newObjectJavaArgs(1000, "thousand", false);
      newObjectJavaArgs(1001, make_jstring("thousand+1"), false);
    }
  }

  // Declaring a register function for each class makes it easy to
  // manage the process.
  static void registerNatives() {
    registerHybrid({
        // Test registration of static method returning void.
        makeNativeMethod("initHybrid", TestHybridClass::initHybrid),
        // overloaded members are ugly to get pointers to.  Use a
        // different name for each C++ method to keep things readable.
        makeNativeMethod("initHybrid", TestHybridClass::initHybrid2),
        makeNativeMethod("initHybrid", TestHybridClass::initHybrid3),
        // C++ members can be registered directly.  The C++ instance
        // created by makeCxxInstance will be retrieved from the
        // HybridData, and the method will be invoked with the
        // arguments passed to java.  These test registration of
        // method pointers returning non-void.
        makeNativeMethod("getInt", TestHybridClass::getInt),
        makeNativeMethod("getString", TestHybridClass::getString),
        makeNativeMethod("getCharString", TestHybridClass::getCharString),
        makeNativeMethod("setBoth", TestHybridClass::setBoth),
        makeNativeMethod("copy1", TestHybridClass::copy1),
        makeNativeMethod("copy2", TestHybridClass::copy2),
        makeNativeMethod("oops", TestHybridClass::oops),
        makeNativeMethod("setGlobal", TestHybridClass::setGlobal),
        makeNativeMethod("getGlobal1", TestHybridClass::getGlobal1),
        makeNativeMethod("getGlobal2", TestHybridClass::getGlobal2),
        makeNativeMethod("makeWithTwo", TestHybridClass::makeWithTwo),
        makeNativeMethod("makeWithThree", TestHybridClass::makeWithThree),
        makeNativeMethod("autoconvertMany", TestHybridClass::autoconvertMany),
      });
  }

private:
  friend HybridBase;

  int i_ = 0;
  std::string s_;
  bool b_ = false;
  global_ref<jstring> global_;
};

class AbstractTestHybrid : public facebook::jni::HybridClass<AbstractTestHybrid> {
public:
  static constexpr const char* const kJavaDescriptor =
    "Lcom/facebook/jni/HybridTests$AbstractTestHybrid;";

  AbstractTestHybrid(int nn)
      : nativeNum_(nn) {}

  int nativeNum() { return nativeNum_; }
  // This is different than the java method!
  virtual int sum() = 0;

  static void registerNatives() {
    registerHybrid({
      makeNativeMethod("nativeNum", AbstractTestHybrid::nativeNum),
    });
  }

 private:
  int nativeNum_;
};

class ConcreteTestHybrid
    : public facebook::jni::HybridClass<ConcreteTestHybrid, AbstractTestHybrid> {
 public:
  static constexpr const char* const kJavaDescriptor =
    "Lcom/facebook/jni/HybridTests$ConcreteTestHybrid;";

  static local_ref<jhybriddata> initHybrid(alias_ref<jclass>, int nn, int cn) {
    return makeCxxInstance(nn, cn);
  }

  int concreteNum() {
    return concreteNum_;
  }
  int sum() override {
    return nativeNum() + concreteNum();
  }

  static void registerNatives() {
    registerHybrid({
      makeNativeMethod("initHybrid", ConcreteTestHybrid::initHybrid),
      makeNativeMethod("concreteNum", ConcreteTestHybrid::concreteNum),
    });
  }

 private:
  friend HybridBase;
  ConcreteTestHybrid(int nn, int cn)
      : HybridBase(nn)
      , concreteNum_(cn) {}

  int concreteNum_;
};

static jboolean cxxTestInheritance(alias_ref<jclass>, AbstractTestHybrid* ath) {
  bool ret = true;

  ret &= ath->nativeNum() == 5;
  ret &= ath->sum() == 11;

  auto cth = dynamic_cast<ConcreteTestHybrid*>(ath);

  ret &= cth != nullptr;
  ret &= cth->concreteNum() == 6;

  return ret;
}

static local_ref<AbstractTestHybrid::jhybridobject> makeAbstractHybrid(alias_ref<jclass>) {
  auto cth = ConcreteTestHybrid::newObjectJavaArgs(0,0,0);
  weak_ref<ConcreteTestHybrid::jhybridobject> wcth = make_weak(cth);
  weak_ref<AbstractTestHybrid::jhybridobject> wath = wcth;

  local_ref<AbstractTestHybrid::jhybridobject> ath = cth;
  return ConcreteTestHybrid::newObjectJavaArgs(cthis(ath)->nativeNum() + 7, 8, 9);
}

struct Base : public JavaClass<Base> {
  static constexpr const char* const kJavaDescriptor = "Lcom/facebook/jni/HybridTests$Base;";
};

class Derived : public HybridClass<Derived, Base> {
 public:
  static constexpr const char* const kJavaDescriptor = "Lcom/facebook/jni/HybridTests$Derived;";

 private:
  friend HybridBase;
  Derived() {}
};

struct Destroyable : public HybridClass<Destroyable> {
 public:
  static constexpr const char* const kJavaDescriptor = "Lcom/facebook/jni/HybridTests$Destroyable;";

  ~Destroyable() override {
    std::lock_guard<std::mutex> lk(*mtx_);
    *dead_ = true;
    cv_->notify_one();
  }

  using HybridBase::makeCxxInstance;

 private:
  friend HybridBase;
  Destroyable(std::shared_ptr<std::mutex> mtx,
      std::shared_ptr<bool> dead,
      std::shared_ptr<std::condition_variable> cv)
    : mtx_(mtx), dead_(dead), cv_(cv) {}

  std::shared_ptr<std::mutex> mtx_;
  std::shared_ptr<bool> dead_;
  std::shared_ptr<std::condition_variable> cv_;
};

static jboolean cxxTestHybridDestruction(alias_ref<jclass>) {
  auto mtx = std::make_shared<std::mutex>();
  auto dead = std::make_shared<bool>(false);
  auto cv = std::make_shared<std::condition_variable>();
  Destroyable::makeCxxInstance(mtx, dead, cv);
  std::unique_lock<std::mutex> lk(*mtx);
  cv->wait(lk, [&] { return dead; });
  return JNI_TRUE;
}


static jboolean cxxTestDerivedJavaClass(alias_ref<jclass>) {
  bool ret = true;

  auto derivedJava = Derived::newObjectCxxArgs();

  ret &= derivedJava ? true : false;
  ret &= derivedJava->isInstanceOf(Derived::javaClassLocal());
  ret &= derivedJava->isInstanceOf(Base::javaClassLocal());

  auto javaPtr = derivedJava.get();

  ret &= dynamic_cast<Derived::javaobject>(javaPtr) != nullptr;
  ret &= dynamic_cast<Base::javaobject>(javaPtr) != nullptr;
  ret &= dynamic_cast<jobject>(javaPtr) != nullptr;

  return ret;
}

class TestHybridClassBase
: public facebook::jni::HybridClass<TestHybridClassBase> {
  public:
    static constexpr const char* const kJavaDescriptor =
      "Lcom/facebook/jni/HybridTests$TestHybridClassBase;";

    TestHybridClassBase() {}

    explicit TestHybridClassBase(int i)
      : i_(i) {}

    static void initHybrid(alias_ref<jhybridobject> o, jint i) {
      // The arguments will be forwarded to the ctor, and the result
      // will be saved in mHybridPointer in the java object.
      setCxxInstance(o, i);
    }

    static void initHybrid2(alias_ref<jhybridobject> o) {
      setCxxInstance(o);
    }

    virtual jint getInt() {
      return i_;
    }

    void setInt(int i) {
      i_ = i;
    }

    static local_ref<jhybridobject> makeWithThree(alias_ref<jclass> /* unused */) {
      // The args are passed to C++ initialization directly.
      return newObjectCxxArgs(3);
    }

    static void registerNatives() {
      registerHybrid({
          makeNativeMethod("initHybrid", TestHybridClassBase::initHybrid),
          makeNativeMethod("initHybrid", TestHybridClassBase::initHybrid2),
          makeNativeMethod("getInt", TestHybridClassBase::getInt),
          makeNativeMethod("setInt", TestHybridClassBase::setInt),
          makeNativeMethod("makeWithThree", TestHybridClassBase::makeWithThree),
          });
    }
  private:
    int i_ = 0;
};

void RegisterTestHybridClass() {
  TestHybridClass::registerNatives();
  AbstractTestHybrid::registerNatives();
  ConcreteTestHybrid::registerNatives();
  TestHybridClassBase::registerNatives();

  registerNatives("com/facebook/jni/HybridTests", {
    makeNativeMethod("cxxTestInheritance", cxxTestInheritance),
    makeNativeMethod("makeAbstractHybrid", makeAbstractHybrid),
    makeNativeMethod("cxxTestDerivedJavaClass", cxxTestDerivedJavaClass),
    makeNativeMethod("cxxTestHybridDestruction", cxxTestHybridDestruction),
  });
}
