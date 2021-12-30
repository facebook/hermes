/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.Arrays;

public class OptionHelpers {

  public enum OptionType {
    BOOLEAN,
    STRING
  }

  public static Object DefaultNumberOption(
      Object value, Object minimum, Object maximum, Object fallback) throws JSRangeErrorException {
    if (JSObjects.isUndefined(value)) return fallback;

    if (!JSObjects.isNumber(value)) throw new JSRangeErrorException("Invalid number value !");

    double d = JSObjects.getJavaDouble(value);
    if (Double.isNaN(d)
        || d > JSObjects.getJavaDouble(maximum)
        || d < JSObjects.getJavaDouble(minimum))
      throw new JSRangeErrorException("Invalid number value !");

    return value;
  }

  public static Object GetNumberOption(
      Object options, String property, Object minimum, Object maximum, Object fallback)
      throws JSRangeErrorException {
    Object value = JSObjects.Get(options, property);
    return DefaultNumberOption(value, minimum, maximum, fallback);
  }

  // https://tc39.es/ecma402/#sec-getoption
  public static Object GetOption(
      Object options, String property, OptionType type, Object validValues, Object fallback)
      throws JSRangeErrorException {
    Object value = JSObjects.Get(options, property);
    if (JSObjects.isUndefined(value)) {
      return fallback;
    }

    // This is wierd. Currently empty string options are passed to java layer as null, hence we do
    // this.
    // [[Follow up]]
    if (JSObjects.isNull(value)) {
      value = "";
    }

    if (type == OptionType.BOOLEAN) {
      if (!JSObjects.isBoolean(value)) {
        throw new JSRangeErrorException("Boolean option expected but not found");
      }
    }

    if (type == OptionType.STRING) {
      if (!JSObjects.isString(value)) {
        throw new JSRangeErrorException("String option expected but not found");
      }
    }

    if (!JSObjects.isUndefined(validValues)) {
      // It must be an array otherwise.
      if (!Arrays.asList((Object[]) validValues).contains(value))
        throw new JSRangeErrorException("String option expected but not found");
    }

    return value;
  }

  public static <T extends Enum<T>> T searchEnum(Class<T> enumeration, Object value) {
    try {

      if (JSObjects.isUndefined(value)) {
        return Enum.valueOf(enumeration, "UNDEFINED");
      }

      if (JSObjects.isNull(value)) {
        return null;
      }

      String valueString = JSObjects.getJavaString(value);
      if (valueString.equals("2-digit")) {
        return Enum.valueOf(enumeration, "DIGIT2");
      }

      for (T each : enumeration.getEnumConstants()) {
        if (each.name().compareToIgnoreCase(valueString) == 0) {
          return each;
        }
      }

      return null;

    } catch (IllegalArgumentException ex) {
      return null;
    }
  }
}
