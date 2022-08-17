/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.HashMap;
import java.util.Map;

public class JSObjects {

  // This is a simulation Javascript like object semantics required for option parsing in Intl
  // Services.
  // The pseudo codes is the Intl spec are described assuming Javascript object semantics, with
  // subtle meaning for undefine, null, empty string etc. which can be very tricky with Java object
  // semantics.
  // We use these object only for option parsing with the help of OptionHelpers class. At the end of
  // the option parsing, all the options are resolved to strongly typed Java datatypes.

  private static class UndefinedObject extends Object {}

  private static class NullObject extends Object {}

  private static Object s_undefined = new UndefinedObject();

  public static Object Undefined() {
    return s_undefined;
  }

  public static boolean isUndefined(Object object) {
    return object instanceof UndefinedObject;
  }

  private static Object s_null = new NullObject();

  public static Object Null() {
    return s_null;
  }

  public static boolean isNull(Object object) {
    return object instanceof NullObject;
  }

  public static boolean isString(Object object) {
    return object instanceof String;
  }

  public static Object newString() {
    return new String();
  }

  public static Object newString(String str) {
    return str;
  }

  public static boolean isBoolean(Object object) {
    return object instanceof Boolean;
  }

  public static Object newBoolean() {
    return new Boolean(false);
  }

  public static Object newBoolean(boolean b) {
    return new Boolean(b);
  }

  public static Object newBoolean(String str) {
    return Boolean.valueOf(str);
  }

  public static boolean getJavaBoolean(Object object) {
    return (Boolean) object;
  }

  public static boolean isNumber(Object object) {
    return object instanceof Double;
  }

  public static Object newNumber(double d) {
    return new Double(d);
  }

  public static double getJavaDouble(Object object) {
    return (Double) object;
  }

  public static String getJavaString(Object object) {
    return (String) object;
  }

  public static boolean isArray(Object object) {
    return object instanceof Object[];
  }

  public static boolean isObject(Object object) {
    return object instanceof HashMap;
  }

  public static Map<String, Object> getJavaMap(Object object) {
    return (HashMap<String, Object>) object;
  }

  public static Object newObject() {
    return new HashMap<String, Object>();
  }

  public static Object Get(Object options, String property) {
    HashMap<String, Object> javaObject = (HashMap<String, Object>) options;
    if (javaObject.containsKey(property)) {
      Object value = javaObject.get(property);
      if (value == null) return JSObjects.Null();
      else return value;
    } else {
      return Undefined();
    }
  }

  public static void Put(Object options, String property, Object value) {
    HashMap<String, Object> javaObject = (HashMap<String, Object>) options;
    javaObject.put(property, value);
  }
}
