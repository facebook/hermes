# fbjni

The Facebook JNI helpers library is designed to simplify usage of the
[Java Native Interface](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html).
The helpers were implemented to ease the integration of cross-platform mobile
code on Android, but there are no Android specifics in the design. It can be
used with any Java VM that supports JNI.

```cpp
struct JMyClass : JavaClass<JMyClass> {
  static constexpr auto kJavaDescriptor = "Lcom/example/MyClass;";

  // Automatic inference of Java method descriptors.
  static std::string concatenate(
      alias_ref<JClass> clazz,
      // Automatic conversion to std::string.
      std::string prefix) {
    // Call methods easily.
    static const auto getSuffix = clazz->getStaticMethod<JString()>("getSuffix");
    // Manage JNI references automatically.
    local_ref<JString> jstr = getSuffix(clazz);
    // Automatic exception translation between Java and C++ (both ways).
    // No need to check exception state after each call.
    result += jstr->toStdString();
    // Automatic conversion from std::string.
    return result;
  }
};
```

## Documentation

- [Why use a JNI wrapper?](docs/rationale.md)
- [Set up your Android build with fbjni](docs/android_setup.md)
- [Quick reference to most features (great for copy/pasting!)](docs/quickref.md)
- [Internal documentation for maintainers](docs/maintainers.md)
<!-- TODO: tutorial -->
<!-- TODO: Comparison with other frameworks. -->

## License

fbjni is Apache-2 licensed, as found in the [LICENSE](/LICENSE) file.
