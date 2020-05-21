/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents the Java part of the Android
 * Intl.NumberFormat implementation.  The interaction with the Hermes
 * JaveScript internals are implemented in C++ and should not
 * generally need to be changed.  Implementers' notes here will
 * describe what parts of the ECMA 402 spec remain to be implemented.
 *
 * Also see the implementer' notes on DateTimeFormat.java.
 */
public class NumberFormat {
  // options are localeMatcher:string, numberingSystem:string, notation:string,
  // compactDisplay:string, useGrouping:string, signDisplay:string
  //
  // Implementer note: The ctor corresponds roughly to
  // https://tc39.es/ecma402/#sec-initializenumberformat
  // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
  public NumberFormat(List<String> locales, Map<String, Object> options) {
  }

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.numberformat.supportedlocalesof
  //
  // The notes on DateTimeFormat#DateTimeFormat() for Locales and
  // Options also apply here.
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) {
    return locales;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.numberformat.prototype.resolvedoptions
  //
  // Also see the implementer notes on DateTimeFormat#resolvedOptions()
  public Map<String, Object> resolvedOptions() {
    return new HashMap<String, Object>();
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatnumber
  public String format(double n) {
    return Double.toString(n);
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatnumbertoparts
  public List<Map<String, String>> formatToParts(double n) {
    ArrayList<Map<String, String>> ret = new ArrayList<Map<String, String>>();
    HashMap<String, String> part = new HashMap<String, String>();
    part.put("type", "integer");
    // This isn't right, but I didn't want to do more work for a stub.
    part.put("value", Double.toString(n));
    ret.add(part);
    return ret;
  }
}

