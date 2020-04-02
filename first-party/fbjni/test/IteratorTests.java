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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.junit.Test;

public class IteratorTests extends BaseFBJniTests {
  @Test
  public void testListIterator() {
    List<String> list = new ArrayList<String>();
    list.add("red");
    list.add("green");
    list.add("blue");

    assertThat(nativeTestListIterator(list)).isTrue();
  }

  private static native boolean nativeTestListIterator(List list);

  @Test
  public void testMapIterator() {
    Map<String, Integer> map = new HashMap<String, Integer>();
    map.put("one", 1);
    map.put("two", 2);
    map.put("four", 4);

    assertThat(nativeTestMapIterator(map)).isTrue();
  }

  private static native boolean nativeTestMapIterator(Map map);

  @Test(expected = ClassCastException.class)
  public void testMapIterateWrongType() {
    Map<String, Number> map = new HashMap<String, Number>();
    map.put("one", 1);
    map.put("two", 2);
    map.put("pi", 3.14);

    assertThat(nativeTestIterateWrongType(map)).isTrue();
  }

  private static native boolean nativeTestIterateWrongType(Map map);

  @Test
  public void testMapIterateNullKey() {
    Map<String, Integer> map = new HashMap<String, Integer>();
    map.put("one", 1);
    map.put(null, -99);
    map.put("four", 4);

    assertThat(nativeTestIterateNullKey(map)).isTrue();
  }

  private static native boolean nativeTestIterateNullKey(Map map);

  @Test
  public void testLargeMapIteration() {
    Map<String, String> map = new HashMap<String, String>();
    for (int i = 0; i < 3000; i++) {
      map.put("" + i, "value");
    }
    assertThat(nativeTestLargeMapIteration(map)).isTrue();
  }

  private static native boolean nativeTestLargeMapIteration(Map map);
}
