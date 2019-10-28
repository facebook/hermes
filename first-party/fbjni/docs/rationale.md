# Why a JNI wrapper?

## Simplified code

Standard JNI is very verbose and error prone. The goal of the fbjni is to make its usage simple, robust and extensible. Below is an example of plain JNI code to call the `toString()` method of an object.

```cpp
// JNI Example: toString()
std::string jniToString(JNIEnv* env, jobject self) {
  static auto cls = env->FindClass("java/lang/Object");
  static auto toStringId =
    env->GetMethodID(cls, "toString", "()Ljava/lang/String;");

  auto jstr = static_cast<jstring>(env->CallObjectMethod(self, toStringId));
  auto chbuf = env->GetStringUTFChars(jstr, nullptr);
  auto stdstr = std::string(chbuf);
  env->ReleaseStringUTFChars(jstr, chbuf);

  return stdstr;
}
```

The code is already verbose and there are some quirks, but most importantly it has no error handling. By adding error handling we get the following code.

```cpp
// JNI Example with error handling: toString()
std::string jniToString(JNIEnv* env, jobject self) {
  static auto cls = env->FindClass("java/lang/Object");
  assert(cls != nullptr);
  static auto id = env->GetMethodID(cls, "toString", "()Ljava/lang/String;");
  assert(id != nullptr);

  auto jstr = static_cast<jstring>(env->CallObjectMethod(self, toStringId));
  throwIfJavaExceptionRaised(env);

  auto chbuf = env->GetStringUTFChars(jstr, nullptr);
  throwIfJavaExceptionRaised(env);

  auto stdstr = std::string(chbuf);
  env->ReleaseStringUTFChars(jstr, chbuf);

  return stdstr;
}
```
In this example we skipped the implementation of `throwIfJavaExceptionRaised()` for the sake of readability. Using fbjni we can greatly reduce the code with robust error handling.

```cpp
// fbjni Example: toString()
std::string helperToString(alias_ref<jobject> obj) {
  static auto toStringMethod =
    findClass("java/lang/Object")->getMethod<jstring()>("toString");

  return toStringMethod(obj)->toStdString();
}
```
Actually, `toString()` is often useful and thus it is made easily available.

```cpp
// fbjni Example (alternative): toString()
std::string helperToString(alias_ref<jobject> obj) {
  return obj->toString();
}
```
The implementations do basically the same things as the plain JNI example. The major difference is that all errors are handled by throwing C++ exceptions. In the plain JNI code some assertions were used to simplify the code.


## JNI is a C API

There is very little syntactic sugar in the API to help you manage resources. And since it is an API specification only, the implementation varies by your JVM — the majority of articles on the web assume HotSpot rather than Dalvik. The documentation is heavily skewed towards the use case where Java calls into a single, self-contained native function (and this is usually a fairly easy case to handle). However, our code is usually asynchronous, requiring callbacks or other stateful interactions between Java and native code.

## Memory Lifecycle

Java and native code have separate memory heaps, so managing the lifecycle of an object that is used on both sides requires extra care. JNI provides the concept of references, which allows native code to tell the JVM that it is still using an object and prevent that object from being garbage collected. When calling from Java, all parameters to the native function are automatically turned into local references, which are dropped when the call completes. To maintain state on the native side or initiate calls into Java, you need to manage these references yourself. References come from fixed size pools, so in addition to leaking the Java objects they point to, leaking or double-freeing references is a problem in and of itself. fbjni provides RAII smart references which tremendously simplify the management of these references.

Conversely, C++ smart pointers only survive as long as C++ code is holding them. Via reinterpret_cast, pointers to heap memory can be stored in Java objects, but you need to be diligent in making sure the C++ allocations are deleted when the Java object becomes unused.  fbjni's Hybrid class provides a convenient way to encapsulate this behavior.

## Exceptions

Most usages of both C++ and Java use exceptions. These two exception frameworks are incompatible, and an exception hitting the JNI boundary therefore becomes a problem: C++ exceptions will cause the program to abort, while Java exceptions require C-style checking for the existence of an error condition. Unhandled Java exceptions will cause the *next* JNI call to abort, which may be completely unrelated to the code that caused the exception in the first place. fbjni provides a translation facility for exceptions that lets them be passed across the boundary in a sane fashion, along with helpers to wrap C++ functions in try/catch blocks.

## Java Type Signatures

Native functions need to specify a stringified version of the Java function signature they correspond to (called a descriptor). This is mechanical; javac can do it for you. However this flow is inconvenient; you can’t use the javah-generated function prototypes as-is, for example, because they assume implicit function registration, which is not reliable on older versions of Android. Getting the descriptor wrong (or forgetting to update it during a refactor) usually results in a very quick but difficult to debug crash. fbjni includes wrappers which deduce the function signature for you based the C++ function prototype.

## Type Safety

JNI provides a single base object type of jobject, with token specializations for jstring, jclass and jexception. fbjni makes it relatively easy to extend this so that any arbitrary Java class can be handled in C++ in a type-safe manner.
