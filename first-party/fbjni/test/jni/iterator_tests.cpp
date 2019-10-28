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

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fbjni/fbjni.h>

#include "expect.h"

using namespace facebook::jni;

namespace {

template <typename K, typename V>
struct JHashMap : public JavaClass<JHashMap<K,V>, JMap<K,V>> {
  constexpr static auto kJavaDescriptor = "Ljava/util/HashMap;";
};

}

jboolean nativeTestListIterator(
    alias_ref<jclass>,
    alias_ref<JList<jstring>> jlist) {
  EXPECT(jlist);

  EXPECT(jlist->size() == 3);

  std::vector<std::string> vs1;
  for (const auto& elem : *jlist) {
    vs1.push_back(elem->toStdString());
  }

  EXPECT(vs1.size() == 3);
  EXPECT(vs1[0] == "red");
  EXPECT(vs1[1] == "green");
  EXPECT(vs1[2] == "blue");

  std::vector<std::string> vs2;
  std::transform(jlist->begin(), jlist->end(), std::back_inserter(vs2),
                 [](local_ref<jstring> elem) { return elem->toStdString(); });

  EXPECT(vs1 == vs2);

  std::vector<std::string> vs3 = { "red", "green", "blue" };

  EXPECT(vs1 == vs3);

  static auto iteratorMethod =
    JIterable<jstring>::javaClassStatic()->getMethod<JIterator<jstring>()>("iterator");
  auto iter = iteratorMethod(jlist);

  EXPECT(std::equal(iter->begin(), iter->end(), jlist->begin()));
  EXPECT(std::equal(iter->begin(), iter->end(), vs3.begin(),
                    [](const local_ref<jstring>& a, const std::string& b) {
                      return a->toStdString() == b;
                    }));

  return JNI_TRUE;
}

jboolean nativeTestMapIterator(
    alias_ref<jclass>,
    alias_ref<JMap<jstring, JInteger>> jmap) {
  EXPECT(jmap);

  EXPECT(jmap->size() == 3);

  std::unordered_map<std::string, int> umap;

  for (const auto& entry : *jmap) {
    umap[entry.first->toStdString()] = entry.second->intValue();
  }

  EXPECT(umap.size() == 3);

  EXPECT(umap["one"] == 1);
  EXPECT(umap["two"] == 2);
  EXPECT(umap["four"] == 4);

  // For an empty map, any types will do; the cast will only happen on null
  // pointers, which will always succeed.
  typedef JHashMap<jclass, jthrowable> TestMap;

  static auto testmapCtor = TestMap::javaClassStatic()->
    getConstructor<TestMap::javaobject()>();
  auto emptyMap = TestMap::javaClassStatic()->newObject(testmapCtor);
  EXPECT(emptyMap->size() == 0);

  JHashMap<jclass, jthrowable>::Iterator i1 = emptyMap->begin();
  JHashMap<jclass, jthrowable>::Iterator i2 = emptyMap->end();

  EXPECT(i1 == i2);

  return JNI_TRUE;
}

jboolean nativeTestIterateWrongType(
    alias_ref<jclass>,
    alias_ref<JMap<jstring, JInteger::javaobject>> jmap) {
  EXPECT(jmap);

  EXPECT(jmap->size() == 3);

  for (const auto& entry : *jmap) {
    (void) entry;
  }

  // The above should throw an exception.
  EXPECT(false);

  return JNI_FALSE;
}

jboolean nativeTestIterateNullKey(
    alias_ref<jclass>,
    alias_ref<JMap<jstring, JInteger>> jmap) {
  EXPECT(jmap);

  EXPECT(jmap->size() == 3);

  std::unordered_map<std::string, int> umap;
  std::unordered_set<int> nullValues;

  for (const auto& entry : *jmap) {
    if (entry.first) {
      umap[entry.first->toStdString()] = entry.second->intValue();
    } else {
      nullValues.insert(entry.second->intValue());
    }
  }

  EXPECT(umap.size() == 2);

  EXPECT(umap["one"] == 1);
  EXPECT(umap["four"] == 4);

  EXPECT(nullValues.size() == 1);
  EXPECT(*nullValues.begin() == -99);

  return JNI_TRUE;
}

jboolean nativeTestLargeMapIteration(
    alias_ref<jclass>,
    alias_ref<JMap<jstring, jstring>> jmap) {
  EXPECT(jmap);
  EXPECT(jmap->size() == 3000);

  for (const auto& entry : *jmap) {
    if (!entry.first) {
      return JNI_FALSE;
    }
  }
  return JNI_TRUE;
}

void RegisterIteratorTests() {
  registerNatives("com/facebook/jni/IteratorTests", {
    makeNativeMethod("nativeTestListIterator", nativeTestListIterator),
    makeNativeMethod("nativeTestMapIterator", nativeTestMapIterator),
    makeNativeMethod("nativeTestIterateWrongType", nativeTestIterateWrongType),
    makeNativeMethod("nativeTestIterateNullKey", nativeTestIterateNullKey),
    makeNativeMethod("nativeTestLargeMapIteration", nativeTestLargeMapIteration),
  });
}
