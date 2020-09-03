/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.util.ULocale;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Intl {
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-canonicalizelocalelist
  //
  // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
  // for more discussion of locales and CanonicalizeLocaleList.
  public static List<String> getCanonicalLocales(List<String> locales)
    throws JSRangeErrorException
  {
    // This implementation is incomplete.  However, it does show that
    // the Android Java ICU methods can successfully be called.

    ArrayList<String> ret = new ArrayList<String>();
    for (String l : locales) {
      String c = ULocale.getName(l);
      if (!ret.contains(c)) {
        ret.add(c);
      }
    }
    return ret;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sup-string.prototype.tolocalelowercase
  public static String toLocaleLowerCase(List<String> locales, String str)
  {
    return "lowered";
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sup-string.prototype.tolocaleuppercase
  public static String toLocaleUpperCase(List<String> locales, String str)
  {
    return "uppered";
  }
}
