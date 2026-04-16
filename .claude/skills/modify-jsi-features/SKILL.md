---
name: modify-jsi-features
description: >
  Guide for adding new JSI functionality to the JavaScript Interface (JSI) layer.
  Use when the user asks to add, create, or implement new methods or features
  in the JSI Runtime interface. Covers all required files across JSI core,
  Hermes implementation, and SynthTrace replay support.
---

# Adding New JSI Functionality

When adding new functionality (methods) to the JSI Runtime interface, you must
modify a specific set of files across multiple layers. This skill describes
each file, the patterns to follow, and important conventions.

## Architecture Overview

JSI (JavaScript Interface) is an abstraction layer that allows C++ code to
interact with JavaScript runtimes. The architecture consists of:

1. **JSI Core** (`xplat/jsi/jsi/`) — The abstract interface definitions
2. **Hermes Implementation** (`xplat/static_h/API/hermes/`) — Hermes-specific implementation
3. **SynthTrace** (`xplat/static_h/API/hermes/`) — Recording and replay infrastructure for debugging

## Checklist of Files to Modify

### JSI Core Files

1. `xplat/jsi/jsi/jsi.h` — Add pure virtual method declaration to `IRuntime` interface AND override declaration in `Runtime` class
2. `xplat/jsi/jsi/jsi.cpp` — Add default implementation (if providing one)
3. `xplat/jsi/jsi/jsi-inl.h` — Add inline helper methods (if needed)
4. `xplat/jsi/jsi/decorator.h` — Add method overrides to `RuntimeDecorator` and `WithRuntimeDecorator`
5. `xplat/jsi/jsi/test/testlib.cpp` — Add tests for the JSI API

### Hermes Implementation Files

6. `xplat/static_h/API/hermes/hermes.cpp` — Add Hermes-specific implementation in `HermesRuntimeImpl`
7. `xplat/static_h/unittests/API/APITest.cpp` — Add Hermes-specific tests

### SynthTrace Files (Required for Hermes)

8. `xplat/static_h/API/hermes/SynthTrace.h` — Add new Record types
9. `xplat/static_h/API/hermes/SynthTrace.cpp` — Implement Record serialization
10. `xplat/static_h/API/hermes/TracingRuntime.h` — Declare tracing method overrides
11. `xplat/static_h/API/hermes/TracingRuntime.cpp` — Implement tracing logic
12. `xplat/static_h/API/hermes/SynthTraceParser.cpp` — Add parsing for new records
13. `xplat/static_h/API/hermes/TraceInterpreter.cpp` — Add replay logic
14. `xplat/static_h/unittests/API/SynthTraceTest.cpp` — Add replay tests
15. `xplat/static_h/unittests/API/SynthTraceSerializationTest.cpp` — Add serialization tests
16. `xplat/static_h/unittests/API/SynthTraceParserTest.cpp` — Add parser tests

## Implementation Strategy

When adding new features (methods) to the JSI Runtime, you have two options:

### Option A: Default Implementation (Recommended)

Provide a default implementation in `jsi::Runtime` that works via JavaScript
calls. This allows all runtimes (JSC, V8, etc.) to work without modification.

```cpp
// jsi.h - Add virtual method with default implementation
virtual void myNewMethod(const Object& obj, const Value& val);

// jsi.cpp - Implement default using JavaScript
void Runtime::myNewMethod(const Object& obj, const Value& val) {
  auto myFn = global()
                  .getPropertyAsObject(*this, "Object")
                  .getPropertyAsFunction(*this, "myMethod");
  myFn.call(*this, obj, val);
}
```

### Option B: Pure Virtual (Requires All Runtime Implementations)

Make the method pure virtual, requiring all runtimes (JSCRuntime, V8Runtime,
HermesRuntime) to implement it. Only use this when a JavaScript-based
default is not possible.

## Step-by-Step Guide

### 1. Add to `jsi.h` (JSI Core Interface)

Add the pure virtual method declaration to the `IRuntime` interface, and the override declaration to the `Runtime` class:

```cpp
// In IRuntime interface (around line 580)
class JSI_EXPORT IRuntime : public ICast {
  // ... existing methods ...

  /// Brief description of what the method does.
  /// \param obj Description of the object parameter.
  /// \param val Description of the value parameter.
  /// \return Description of return value (if any).
  virtual void myNewMethod(const Object& obj, const Value& val) = 0;

  // For methods returning Value, use this pattern:
  virtual Value myNewGetter(const Object& obj) = 0;
};

// In Runtime class (around line 730) - add override declarations
class JSI_EXPORT Runtime : public IRuntime {
  // ... existing methods ...

  void myNewMethod(const Object& obj, const Value& val) override;
  Value myNewGetter(const Object& obj) override;
};
```

**Important**: The `IRuntime` interface contains the pure virtual declarations (`= 0`),
while the `Runtime` class provides the default implementation with `override` keyword.
This separation allows other runtimes to implement the interface differently while
`Runtime` provides a JavaScript-based fallback.
```

If adding convenience methods to JSI types (like `Object`), add them too:

```cpp
class JSI_EXPORT Object : public Pointer {
  // ... existing methods ...

  /// Sets something on the object.
  void setMyThing(IRuntime& runtime, const Value& val) const {
    return runtime.myNewMethod(*this, val);
  }

  /// Gets something from the object.
  inline Value getMyThing(Runtime& runtime) const;
};
```

### 2. Add Default Implementation to `jsi.cpp`

Implement the default using JavaScript APIs when possible:

```cpp
void Runtime::myNewMethod(const Object& obj, const Value& val) {
  // Get the JavaScript function
  auto myMethodFn = global()
                        .getPropertyAsObject(*this, "Object")
                        .getPropertyAsFunction(*this, "myMethod");
  // Call it with the appropriate arguments
  myMethodFn.call(*this, obj, val);
}

Value Runtime::myNewGetter(const Object& obj) {
  auto myGetterFn = global()
                        .getPropertyAsObject(*this, "Object")
                        .getPropertyAsFunction(*this, "myGetter");
  return myGetterFn.call(*this, obj);
}
```

### 3. Add Inline Methods to `jsi-inl.h` (If Needed)

For methods that return incomplete types at declaration time:

```cpp
Value Object::getMyThing(Runtime& runtime) const {
  return runtime.myNewGetter(*this);
}
```

### 4. Update `decorator.h`

Add method overrides to both `RuntimeDecorator` and `WithRuntimeDecorator`:

```cpp
// In RuntimeDecorator class (around line 282)
void myNewMethod(const Object& obj, const Value& val) override {
  plain_.myNewMethod(obj, val);
}

Value myNewGetter(const Object& obj) override {
  return plain_.myNewGetter(obj);
}

// In WithRuntimeDecorator class (around line 760)
void myNewMethod(const Object& obj, const Value& val) override {
  Around around{with_};
  RD::myNewMethod(obj, val);
}

Value myNewGetter(const Object& obj) override {
  Around around{with_};
  return RD::myNewGetter(obj);
}
```

### 5. Add JSI Core Tests in `test/testlib.cpp`

Test the default implementation using a RuntimeDecorator:

```cpp
TEST_P(JSITest, MyNewFeature) {
  // Use a RuntimeDecorator to test the default implementation
  class RD : public RuntimeDecorator<Runtime, Runtime> {
   public:
    explicit RD(Runtime& rt) : RuntimeDecorator(rt) {}

    void myNewMethod(const Object& obj, const Value& val) override {
      return Runtime::myNewMethod(obj, val);
    }

    Value myNewGetter(const Object& obj) override {
      return Runtime::myNewGetter(obj);
    }
  };

  RD rd = RD(rt);
  Object obj(rd);

  // Test the functionality
  obj.setMyThing(rd, Value(123));
  EXPECT_EQ(obj.getMyThing(rd).getNumber(), 123);
}
```

### 6. Add Hermes Implementation in `hermes.cpp`

Implement the optimized version using Hermes VM APIs inside `HermesRuntimeImpl`:

```cpp
// In HermesRuntimeImpl class declaration (around line 673)
void myNewMethod(const jsi::Object& obj, const jsi::Value& val) override;
jsi::Value myNewGetter(const jsi::Object& obj) override;

// Implementation (around line 2074)
void HermesRuntimeImpl::myNewMethod(
    const jsi::Object& obj,
    const jsi::Value& val) {
  // Validate input if needed
  if (!val.isObject() && !val.isNull()) {
    throw jsi::JSError(*this, "Invalid argument type");
  }

  // Use Hermes VM APIs directly
  auto cr = vm::JSObject::myVMMethod(
      vm::vmcast<vm::JSObject>(phv(obj)),
      runtime_,
      hvFromValue(val),
      vm::PropOpFlags().plusThrowOnError());
  checkStatus(cr.getStatus());
}

jsi::Value HermesRuntimeImpl::myNewGetter(const jsi::Object& obj) {
  vm::CallResult<vm::PseudoHandle<vm::JSObject>> cr =
      vm::JSObject::myVMGetter(handle(obj), runtime_);
  checkStatus(cr.getStatus());
  if (!*cr) {
    return jsi::Value::null();
  }
  return valueFromHermesValue(cr->getHermesValue());
}
```

### 7. Add Hermes Tests in `APITest.cpp`

```cpp
TEST_P(HermesRuntimeTest, MyNewFeature) {
  Object obj(*rt);

  // Test normal operation
  Value val(*rt, 123);
  obj.setMyThing(*rt, val);
  EXPECT_EQ(obj.getMyThing(*rt).getNumber(), 123);

  // Test edge cases
  obj.setMyThing(*rt, Value::null());
  EXPECT_TRUE(obj.getMyThing(*rt).isNull());

  // Test error cases
  EXPECT_THROW(obj.setMyThing(*rt, Value("invalid")), JSError);
}
```

### 8. Add SynthTrace Record Types in `SynthTrace.h`

Add new record types for tracing:

```cpp
// In the RECORD macro list (around line 210)
RECORD(MyNewMethod)                    \
RECORD(MyNewGetter)                    \

// Define the record structs (after existing records)

/// A MyNewMethodRecord is an event where native code calls myNewMethod
struct MyNewMethodRecord : public Record {
  static constexpr RecordType type{RecordType::MyNewMethod};
  const ObjectID objID_;
  const TraceValue value_;

  MyNewMethodRecord(TimeSinceStart time, ObjectID objID, TraceValue value)
      : Record(time), objID_(objID), value_(value) {}

  void toJSONInternal(::hermes::JSONEmitter &json) const override;

  RecordType getType() const override {
    return type;
  }

  std::vector<ObjectID> uses() const override {
    std::vector<ObjectID> uses{objID_};
    pushIfTrackedValue(value_, uses);
    return uses;
  }
};

/// A MyNewGetterRecord is an event where native code calls myNewGetter
struct MyNewGetterRecord : public Record {
  static constexpr RecordType type{RecordType::MyNewGetter};
  const ObjectID objID_;

  MyNewGetterRecord(TimeSinceStart time, ObjectID objID)
      : Record(time), objID_(objID) {}

  void toJSONInternal(::hermes::JSONEmitter &json) const override;

  RecordType getType() const override {
    return type;
  }

  std::vector<ObjectID> uses() const override {
    return {objID_};
  }
};
```

### 9. Implement Record Serialization in `SynthTrace.cpp`

```cpp
void SynthTrace::MyNewMethodRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
  json.emitKeyValue("value", encode(value_));
}

void SynthTrace::MyNewGetterRecord::toJSONInternal(JSONEmitter &json) const {
  Record::toJSONInternal(json);
  json.emitKeyValue("objID", objID_);
}
```

### 10. Add Tracing Method Declarations in `TracingRuntime.h`

```cpp
class TracingRuntime : public jsi::RuntimeDecorator<jsi::Runtime> {
  // ... existing methods ...

  void myNewMethod(const jsi::Object& obj, const jsi::Value& val) override;
  jsi::Value myNewGetter(const jsi::Object& obj) override;
```

### 11. Implement Tracing Logic in `TracingRuntime.cpp`

```cpp
void TracingRuntime::myNewMethod(
    const jsi::Object& obj,
    const jsi::Value& val) {
  trace_.emplace_back<SynthTrace::MyNewMethodRecord>(
      getTimeSinceStart(), useObjectID(obj), useTraceValue(val));
  RD::myNewMethod(obj, val);
}

jsi::Value TracingRuntime::myNewGetter(const jsi::Object& obj) {
  trace_.emplace_back<SynthTrace::MyNewGetterRecord>(
      getTimeSinceStart(), useObjectID(obj));

  auto result = RD::myNewGetter(obj);
  trace_.emplace_back<SynthTrace::ReturnToNativeRecord>(
      getTimeSinceStart(), defTraceValue(result));
  return result;
}
```

### 12. Add Parsing in `SynthTraceParser.cpp`

```cpp
case RecordType::MyNewMethod: {
  trace.emplace_back<SynthTrace::MyNewMethodRecord>(
      timeFromStart,
      objID->getValue(),
      SynthTrace::decode(propValue->c_str()));
  break;
}
case RecordType::MyNewGetter:
  trace.emplace_back<SynthTrace::MyNewGetterRecord>(
      timeFromStart, objID->getValue());
  break;
```

### 13. Add Replay Logic in `TraceInterpreter.cpp`

```cpp
case RecordType::MyNewMethod: {
  const auto &record =
      static_cast<const SynthTrace::MyNewMethodRecord &>(*rec);
  auto obj = getJSIValueForUse(record.objID_).getObject(rt_);
  obj.setMyThing(rt_, traceValueToJSIValue(record.value_));
  break;
}
case RecordType::MyNewGetter: {
  const auto &record =
      static_cast<const SynthTrace::MyNewGetterRecord &>(*rec);
  auto obj = getJSIValueForUse(record.objID_).getObject(rt_);
  auto result = obj.getMyThing(rt_);
  retval = std::move(result);
  break;
}
```

### 14. Add SynthTrace Tests

**In `SynthTraceSerializationTest.cpp`:**

```cpp
TEST_F(SynthTraceSerializationTest, MyNewMethodTest) {
  EXPECT_EQ(
      R"({"type":"MyNewMethodRecord","time":0,"objID":1,"value":"null:"})",
      to_string(SynthTrace::MyNewMethodRecord(
          dummyTime, 1, SynthTrace::encodeNull())));
}

TEST_F(SynthTraceSerializationTest, MyNewGetterTest) {
  EXPECT_EQ(
      R"({"type":"MyNewGetterRecord","time":0,"objID":1})",
      to_string(SynthTrace::MyNewGetterRecord(dummyTime, 1)));
}
```

**In `SynthTraceTest.cpp`:**

```cpp
TEST_F(SynthTraceReplayTest, MyNewFeatureReplay) {
  {
    auto &rt = *traceRt;
    jsi::Object obj(rt);
    obj.setMyThing(rt, jsi::Value(123));
    rt.global().setProperty(rt, "obj", obj);

    auto result = obj.getMyThing(rt);
    rt.global().setProperty(rt, "result", result);
  }
  replay();
  {
    auto &rt = *replayRt;
    auto obj = rt.global().getProperty(rt, "obj").getObject(rt);
    EXPECT_EQ(obj.getMyThing(rt).getNumber(), 123);

    auto result = rt.global().getProperty(rt, "result");
    EXPECT_EQ(result.getNumber(), 123);
  }
}
```

**In `SynthTraceParserTest.cpp`:**

```cpp
TEST_F(SynthTraceParserTest, ParseMyNewMethodRecord) {
  const char *src = R"(
{
  "version": 5,
  "globalObjID": 258,
  "runtimeConfig": {
    "gcConfig": {
      "initHeapSize": 33554432,
      "maxHeapSize": 536870912
    }
  },
  "trace": [
    {
      "type": "MyNewMethodRecord",
      "time": 1234,
      "objID": 1,
      "value": "number:123"
    }
  ]
}
  )";
  auto parseResult = parseSynthTrace(bufFromStr(src));
  SynthTrace &trace = std::get<0>(parseResult);

  auto record = dynamic_cast<const SynthTrace::MyNewMethodRecord &>(
      *trace.records().at(0));
  ASSERT_EQ(record.objID_, 1);
}
```

## Key Conventions

1. **Default Implementations**: Always provide a default implementation in
   `jsi::Runtime` when possible. This allows JSCRuntime and V8Runtime to work
   without modification.

2. **Hermes Optimization**: For Hermes, override the default with an optimized
   implementation using Hermes VM APIs directly.

3. **SynthTrace is Required**: For Hermes, you MUST add SynthTrace support.
   SynthTrace allows recording and replaying of JSI interactions, which is
   critical for debugging and testing. Without it, replays will fail.

4. **Consistent Naming**: Use consistent naming across all files. If the method
   is `setPrototypeOf` in JSI, use `SetPrototype` for the record type name.

5. **Error Handling**: Use `jsi::JSError` for JavaScript-level errors in JSI
   code. Use `checkStatus()` for Hermes VM call results.

6. **Testing**: Test at multiple levels:
   - JSI core tests verify the abstract interface and default implementation
   - Hermes API tests verify the optimized implementation
   - SynthTrace tests verify recording, serialization, parsing, and replay

## Testing

### Running JSI Tests

To run all JSI tests defined in `testlib.cpp`:

```bash
buck test xplat/jsi:jsi
```

This runs the JSI test suite against multiple runtime implementations (Hermes, StaticHermes, etc.).

### Running Hermes API Tests

To run the Hermes-specific API tests:

```bash
buck test xplat/static_h:HermesUnitTests
```

To run a specific test:

```bash
buck test xplat/static_h:HermesUnitTests -- MyTestName
```

## Reference Diffs

For a complete example of adding JSI functionality, refer to these diffs:

- **D66562549**: JSI core changes for `getPrototypeOf` / `setPrototypeOf`
- **D66729169**: Hermes implementation
- **D66792338**: SynthTrace support

Note: These diffs predate the `IRuntime` interface, but the patterns remain the
same.
