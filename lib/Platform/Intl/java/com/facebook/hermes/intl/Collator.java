/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents the Java part of the Android Intl.Collator
 * implementation.  The interaction with the Hermes JaveScript
 * internals are implemented in C++ and should not generally need to
 * be changed.  Implementers' notes here will describe what parts of
 * the ECMA 402 spec remain to be implemented.
 *
 * Also see the implementer' notes on DateTimeFormat.java.
 */
public class Collator {
  private Map<String, Object> options;

  // options are usage:string, localeMatcher:string, numeric:boolean, caseFirst:string,
  // sensitivity:string, ignorePunctuation:boolean
  //
  // Implementer note: The ctor corresponds roughly to
  // https://tc39.es/ecma402/#sec-initializecollator
  // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
  public Collator(List<String> locales, Map<String, Object> options)
    throws JSRangeErrorException
  {
    this.options = new HashMap<String, Object>();
    this.options.put("locale", "en-US");
    this.options.put("numeric", Boolean.FALSE);
  }

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.collator.supportedlocalesof
  //
  // The notes on DateTimeFormat#DateTimeFormat() for Locales and
  // Options also apply here.
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options)
    throws JSRangeErrorException
  {
    return java.util.Arrays.asList("en-CA", "de-DE");
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.collator.prototype.resolvedoptions
  //
  // Also see the implementer notes on DateTimeFormat#resolvedOptions()
  public Map<String, Object> resolvedOptions() {
    return options;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-collator-comparestrings
  public double compare(String a, String b) {
    return a.compareTo(b);
  }
}

