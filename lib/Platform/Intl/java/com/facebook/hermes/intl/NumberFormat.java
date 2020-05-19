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

public class NumberFormat {
  // options are localeMatcher:string, numberingSystem:string, notation:string,
  // compactDisplay:string, useGrouping:string, signDisplay:string
  public NumberFormat(List<String> locales, Map<String, Object> options) {
  }

  // options are localeMatcher:string
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) {
    return locales;
  }

  // https://tc39.es/ecma402/#sec-intl.numberformat.prototype.resolvedoptions
  public Map<String, Object> resolvedOptions() {
    return new HashMap<String, Object>();
  }

  public String format(double n) {
    return Double.toString(n);
  }

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

