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

// This is woefully inadequate implementation.
public class Collator {
  private Map<String, Object> options;

  // options are usage:string, localeMatcher:string, numeric:boolean, caseFirst:string,
  // sensitivity:string, ignorePunctuation:boolean
  public Collator(List<String> locales, Map<String, Object> options)
    throws JSRangeErrorException
  {
    this.options = new HashMap<String, Object>();
    this.options.put("locale", "en-US");
    this.options.put("numeric", Boolean.FALSE);
  }

  // options are localeMatcher:string
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options)
    throws JSRangeErrorException
  {
    return java.util.Arrays.asList("en-CA", "de-DE");
  }

  // https://tc39.es/ecma402/#sec-intl.collator.prototype.resolvedoptions
  public Map<String, Object> resolvedOptions() {
    return options;
  }

  public double compare(String a, String b) {
    return a.compareTo(b);
  }
}

