# Quick Reference
- [JavaClass definition and method registration](#javaclass-definition-and-method-registration)
- [Wrapping constructors](#wrapping-constructors)
- [Basic method usage (Java to C++ and C++ to Java)](#basic-method-usage-java-to-c-and-c-to-java)
- [Methods using Java primitives](#methods-using-java-primitives)
- [Methods using Java Strings](#methods-using-java-strings)
- [Working with arrays of primitives](#working-with-arrays-of-primitives)
- [Working with arrays of objects](#working-with-arrays-of-objects)
- [References](#references)
- [Defining a nested class](#defining-a-nested-class)
- [Defining a child class](#defining-a-child-class)
- [Getting and setting fields](#getting-and-setting-fields)
- [Working with JObject and JClass](#working-with-jobject-and-jclass)
- [Catching and throwing exceptions](#catching-and-throwing-exceptions)
- [Working with boxed primitives](#working-with-boxed-primitives)
- [Working with Iterables](#working-with-iterables)
- [Building Collections](#building-collections)
- [Transferring data with direct ByteBuffer](#transferring-data-with-direct-bytebuffer)
## JavaClass definition and method registration
```cpp
#include <fbjni/fbjni.h>
using namespace facebook::jni;
```
```cpp
// Standard declaration for a normal class (no C++ fields).
struct DocTests : JavaClass<DocTests> {
  static constexpr auto kJavaDescriptor = "Lcom/facebook/jni/DocTests;";
```
```cpp
  // NOTE: The name of this method doesn't matter.
  static void registerNatives() {
    javaClassStatic()->registerNatives({
      makeNativeMethod("nativeVoidMethod", DocTests::nativeVoidMethod),
      makeNativeMethod("staticNativeVoidMethod", DocTests::staticNativeVoidMethod),
```
```cpp
jint JNI_OnLoad(JavaVM* vm, void*) {
  return facebook::jni::initialize(vm, [] {
      DocTests::registerNatives();
  });
}
```


## Wrapping constructors
```java
class DataHolder {
  int i;
  String s;
  DataHolder(int i, String s) {
    this.i = i;
    this.s = s;
  }
  static DataHolder someInstance;
}
```
```cpp
struct JDataHolder : JavaClass<JDataHolder> {
  static constexpr auto kJavaDescriptor = "Lcom/facebook/jni/DataHolder;";
  // newInstance should be wrapped to ensure compile-time checking of call sites.
  static local_ref<JDataHolder> create(int i, std::string const& s) {
    // Constructor is looked up by argument types at *runtime*.
    return newInstance(i, s);
  }
```
```cpp
  // Call-site in another file.
  static local_ref<JDataHolder> runConstructor(
      alias_ref<JClass> clazz) {
    // Call to ordinatry C++ function is checked at *compile time*.
    return JDataHolder::create(1, "hi");
  }
```


## Basic method usage (Java to C++ and C++ to Java)
```java
  native void nativeVoidMethod();
  static native void staticNativeVoidMethod();
  void voidMethod() {}
  static void staticVoidMethod() {}
```
```cpp
 public:
  // Java methods should usually be wrapped by C++ methods for ease-of-use.
  // (Most other examples in this document will inline these for brevity.)
  void callVoidMethod() {
    static const auto method = getClass()->getMethod<void()>("voidMethod");
    // self() returns the raw JNI reference to this object.
    method(self());
  }
  static void callStaticVoidMethod() {
    static const auto cls = javaClassStatic();
    static const auto method = cls->getStaticMethod<void()>("staticVoidMethod");
    method(cls);
  }

  // Native implementations of Java methods can be private.
 private:
  // For non-Hybrid objects, all JNI methods must be static on the C++ side
  // because only Hybrid objects can have C++ state.
  static void nativeVoidMethod(
      // All non-static methods receive "this" as a first argument.
      alias_ref<DocTests> thiz) {
    // Make sure we got the right object.
    assert(thiz->toString() == "instance of DocTests");
    thiz->callVoidMethod();
  }
  static void staticNativeVoidMethod(
      // All static methods receive the class as a first argument.
      alias_ref<JClass> clazz) {
    assert(clazz->toString() == "class com.facebook.jni.DocTests");
    DocTests::callStaticVoidMethod();
  }
```


## Methods using Java primitives
```java
  static native long addSomeNumbers(byte b, short s, int i);
  static long doubler(int i) { return i + i; }
```
```cpp
  static jlong addSomeNumbers(alias_ref<JClass> clazz, jbyte b, jshort s, jint i) {
    static const auto doubler = clazz->getStaticMethod<jlong(jint)>("doubler");
    jlong l = doubler(clazz, 4);
    return b + s + i + l;
  }

```
  Argument and return types can be in either JNI style or C++ style.

  | Java type | JNI types |
  | --- | --- |
  | `boolean` | `jboolean`, `bool` |
  | `byte` | `jbyte`, `int8_t` |
  | `char` | `jchar` |
  | `short` | `jshort`, `short`, `int16_t` |
  | `int` | `jint`, `int`, `int32_t` |
  | `long` | `jlong`, `int64_t` |
  | `float` | `jfloat`, `float` |
  | `double` | `jdouble`, `double` |


## Methods using Java Strings
```java
  // Java methods used by the C++ code below.
  static native String fancyCat(String s1, String s2);
  static native String getCString();
  static String doubler(String s) { return s + s; }
```
```cpp
  static std::string fancyCat(
      alias_ref<JClass> clazz,
      // Native methods can receive strings as JString (direct JNI reference) ...
      alias_ref<JString> s1,
      // or as std::string (converted to real UTF-8).
      std::string s2) {
    // Convert JString to std::string.
    std::string result = s1->toStdString();
    // Java methods can receive and return JString ...
    static const auto doubler_java = clazz->getStaticMethod<JString(JString)>("doubler");
    result += doubler_java(clazz, *s1)->toStdString();
    // and also std::string (converted from real UTF-8).
    static const auto doubler_std = clazz->getStaticMethod<std::string(std::string)>("doubler");
    result += doubler_std(clazz, s2)->toStdString();
    // They can also receive const char*, but not return it.
    static const auto doubler_char = clazz->getStaticMethod<std::string(const char*)>("doubler");
    result += doubler_char(clazz, s2.c_str())->toStdString();
    // All 3 formats can be returned (std::string shown here, const char* below).
    return result;
  }

  static const char* getCString(alias_ref<JClass>) {
    // This string is converted to JString *after* getCString returns.
    // Watch your memory lifetimes.
    return "Watch your memory.";
  }
```


## Working with arrays of primitives
```java
  static native int[] primitiveArrays(int[] arr);
```
```cpp
  static local_ref<JArrayInt> primitiveArrays(
      alias_ref<JClass> clazz,
      // JArrayX is available for all primitives.
      alias_ref<JArrayInt> arr) {
    size_t size = arr->size();
    std::vector<jint> buffer(size + 1L);
    // Copy elements into native memory.
    arr->getRegion(0, size, buffer.data());
    // Copy elements into fresh memory (returns unique_ptr<int[]>).
    auto elements = arr->getRegion(0, size);
    // Pin can eliminate the need for a copy.
    {
      auto pin = arr->pin();
      for (size_t i = 0; i < pin.size(); i++) {
        // Can read and/or write pin[i].
        buffer[size] += pin[i];
      }
    }
    // Allocating a new array and copying data in.
    // (Data can also be assigned by writing to a pin.)
    auto ret = JArrayInt::newArray(size + 1);
    ret->setRegion(0, size + 1, buffer.data());
    return ret;
  }
```


## Working with arrays of objects
```cpp
  static local_ref<JArrayClass<JString>> classArrays(
      alias_ref<JClass> clazz,
      alias_ref<JArrayClass<JDataHolder>> arr) {
    size_t size = arr->size();
    local_ref<JArrayClass<JString>> ret = JArrayClass<JString>::newArray(size);
    for (int i = 0; i < size; ++i) {
      local_ref<JString> str = arr->getElement(i)->getStr();
      ret->setElement(i, *str);
    }
    return ret;
  }
```


## References

  ### `alias_ref<JFoo>`
  `alias_ref` is a non-owning reference, like a bare pointer.
  It is used almost exclusively for function arguments.

  ### `local_ref<JFoo>`
  `local_ref` is a ref-counted thread-specific pointer that is invalidated upon returning to Java.
  For variables used within a function, use `local_ref`.
  Most functions should return `local_ref` (and let the caller convert to a `global_ref` if necessary).

  ### `global_ref<JFoo>`
  `global_ref` is a ref-counted pointer.
  Use this for storing a reference to a Java object that may
  outlive the current call from Java into C++
  (e.g. class member fields are usually global refs).
  You can create a new `global_ref` (from an `alias_ref`/`local_ref`) by calling `make_global`.
```cpp
  static local_ref<JObject> convertReferences(
      alias_ref<JClass> clazz,
      alias_ref<JMyDerivedClass> derived) {
    local_ref<JMyDerivedClass> local_derived = make_local(derived);
    global_ref<JMyDerivedClass> global_derived = make_global(derived);
    // Store global_derived somewhere.
    return local_derived;
  }
```


## Defining a nested class
```java
class Outer {
  class Nested {}
}
```
```cpp
struct JNested : JavaClass<JNested> {
  static constexpr auto kJavaDescriptor = "Lcom/facebook/jni/Outer$Nested;";
  static local_ref<JNested> create() {
    return newInstance();
  }
};
```


## Defining a child class
```java
class MyBaseClass {}
class MyDerivedClass extends MyBaseClass {}
```
```cpp
struct JMyBaseClass : JavaClass<JMyBaseClass> {
  static constexpr auto kJavaDescriptor = "Lcom/facebook/jni/MyBaseClass;";
};

struct JMyDerivedClass : JavaClass<JMyDerivedClass, JMyBaseClass> {
  static constexpr auto kJavaDescriptor = "Lcom/facebook/jni/MyDerivedClass;";
};

```
This will allow implicit casts from Derived to Base and explicit downcasts.
When no base class is given, JObject will be used as the base.
```cpp
  static void castReferences(
      alias_ref<JClass> clazz,
      alias_ref<JMyBaseClass> base) {
    // Just like raw pointers, upcasting is implicit.
    alias_ref<JObject> obj = base;
    // static_ref_cast is like C++ static_cast.  No runtime checking is done.
    alias_ref<JMyDerivedClass> derived_1 = static_ref_cast<JMyDerivedClass>(base);
    // dynamic_ref_cast is like C++ dynamic_cast.
    // It will check that the runtime Java type is actually derived from the target type.
    try {
      alias_ref<JMyDerivedClass> derived_2 = dynamic_ref_cast<JMyDerivedClass>(base);
      (void)derived_2;
    } catch (const JniException& exn) {
      // Throws ClassCastException if the cast fails.
      throw;
    }
```


## Getting and setting fields
```cpp
  void getAndSetFields() {
    static const auto cls = javaClassStatic();
    // Primitive fields.
    static const auto iField = cls->getField<jint>("i");
    jint i = this->getFieldValue(iField);
    this->setFieldValue(iField, i + 1);
    // Object fields work for standard classes and your own JavaObject classes.
    static const auto sField = cls->getField<JString>("s");
    // Object are returned as local refs ...
    local_ref<JString> s = this->getFieldValue(sField);
    // and can be set from any ref.
    this->setFieldValue(sField, make_jstring(s->toStdString() + "1").get());
    // Static fields work the same, but getStaticField, getStaticFieldValue,
    // and setStaticFieldValue must all be called on the class object.
    static const auto someInstanceField = cls->getStaticField<JDataHolder>("someInstance");
    auto inst = cls->getStaticFieldValue(someInstanceField);
    if (!inst) {
      // NOTE: Can't use cls here because it is declared const.
      getClass()->setStaticFieldValue(someInstanceField, self());
    }
  }
```


## Working with JObject and JClass
```cpp
  static std::string showJObject(
      alias_ref<JClass> clazz,
      // JObject is the base class of all fbjni types.  It corresponds to java.lang.Object.
      alias_ref<JObject> obj,
      alias_ref<JDataHolder> data) {
    local_ref<JClass> objClass = obj->getClass();
    local_ref<JClass> dataClass = data->getClass();
    local_ref<JClass> parent = dataClass->getSuperclass();
    assert(isSameObject(parent, objClass));
    assert(data->isInstanceOf(parent));
    assert(objClass->isAssignableFrom(clazz));
    std::string str = "data=";
    {
      // Acquires the object lock until this object goes out of scope.
      auto lock = data->lock();
      // Calls Object.toString and converts to std::string.
      str += data->toString();
    }
    // All JavaClass types have a `javaobject` typedef, which is their raw JNI type.
    static_assert(std::is_same<JObject::javaobject, jobject>::value, "");
    static_assert(std::is_same<JClass::javaobject, jclass>::value, "");
    static_assert(!std::is_same<JDataHolder::javaobject, jobject>::value, "");
    static_assert(std::is_convertible<JDataHolder::javaobject, jobject>::value, "");
    return str;
  }
```


## Catching and throwing exceptions
```cpp
  static void catchAndThrow(
      alias_ref<JClass> clazz) {
    try {
      clazz->getStaticMethod<void()>("doesNotExist");
      assert(!"Exception wasn't thrown.");
    } catch (JniException& exn) {
      // JniException extends std::exception, so "catch (std::exception& exn)" also works.
      local_ref<JThrowable> underlying = exn.getThrowable();
      const char* msg = exn.what();
      // Throwing exceptions from C++ is fine.
      // They will be translated to an appropriate Java exception type.
      throw std::runtime_error(std::string() + "Caught '" + msg + "'");
    }
  }
```


## Working with boxed primitives
```java
  static native Double scaleUp(Integer number);
```
```cpp
  static local_ref<JDouble> scaleUp(
      alias_ref<JClass> clazz,
      alias_ref<JInteger> number) {
    // Boxed types exist for all Java primitive types.
    // Unbox with ->value() or ->intValue.
    jint unboxed = number->value();
    jdouble scaled = unboxed * 1.5;
    // Box with autobox() or JDouble::valueOf.
    local_ref<JDouble> ret = autobox(scaled);
    return ret;
  }
```


## Working with Iterables
```java
  static native String concatMatches(List<Integer> values, Map<String, Integer> names);
```
```cpp
  static std::string concatMatches(
      alias_ref<JClass> clazz,
      // Note that generic types are *not* checked against Java declarations.
      alias_ref<JList<JInteger>> values,
      alias_ref<JMap<JString, JInteger>> names) {
    int sum = 0;
    std::string ret;
    // Iterator and Iterable support C++ iteration.
    // Collection, List, and Set support iteration and ->size().
    for (const auto& elem : *values) {
      sum += elem->value();
    }
    // Maps iterate like C++ maps.
    for (const auto& entry : *names) {
      if (entry.second->value() == sum) {
        ret += entry.first->toStdString();
      }
    }
    // This works if you build with C++17.
    // for (const auto& [key, value] : *names) {
    return ret;
  }
```


## Building Collections
```java
  static native Map<String, List<Integer>> buildCollections();
```
```cpp
  static local_ref<JMap<JString, JList<JInteger>>> buildCollections(
      alias_ref<JClass> clazz) {
    auto primes = JArrayList<JInteger>::create();
    primes->add(autobox(2));
    primes->add(autobox(3));
    auto wrapper = JHashMap<JString, JList<JInteger>>::create();
    wrapper->put(make_jstring("primes"), primes);
    return wrapper;
  }
```


## Transferring data with direct ByteBuffer
```java
  static native ByteBuffer transformBuffer(ByteBuffer data);
  static void receiveBuffer(ByteBuffer buffer) {
    assertThat(buffer.capacity()).isEqualTo(2);
    assertThat(buffer.get(0)).isEqualTo((byte)2);
    assertThat(buffer.get(1)).isEqualTo((byte)3);
  }
  @Test
  public void testByteBuffers() {
    ByteBuffer data = ByteBuffer.allocateDirect(2);
    data.put(new byte[] {1, 2});
    ByteBuffer transformed = transformBuffer(data);
    receiveBuffer(transformed);
  }
```
```cpp
#include <fbjni/ByteBuffer.h>
```
```cpp
  static local_ref<JByteBuffer> transformBuffer(
      alias_ref<JClass> clazz,
      alias_ref<JByteBuffer> data) {
    // Direct ByteBuffers are an efficient way to transfer bulk data between Java and C++.
    if (!data->isDirect()) {
      throw std::runtime_error("Argument is not a direct buffer.");
    }
    // Transform data into a local buffer.
    std::vector<uint8_t> buffer(data->getDirectSize());
    uint8_t* raw_data = data->getDirectBytes();
    for (size_t i = 0; i < buffer.size(); ++i) {
      buffer[i] = raw_data[i] + 1;
    }
    // Wrap our data in a buffer and pass to Java.
    // Note that the buffer *directly* references our memory.
    local_ref<JByteBuffer> wrapper = JByteBuffer::wrapBytes(buffer.data(), buffer.size());
    static const auto receiver = clazz->getStaticMethod<void(alias_ref<JByteBuffer>)>("receiveBuffer");
    receiver(clazz, wrapper);
    // We can create a new buffer that owns its own memory and safely return it.
    local_ref<JByteBuffer> ret = JByteBuffer::allocateDirect(buffer.size());
    std::memcpy(ret->getDirectBytes(), buffer.data(), buffer.size());
    return ret;
  }
```


