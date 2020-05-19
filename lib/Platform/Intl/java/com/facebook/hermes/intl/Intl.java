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
  public static List<String> getCanonicalLocales(List<String> locales)
    throws JSRangeErrorException
  {
    // This implementation is incomplete.  It omits step 7.c.v of
    // https://tc39.es/ecma402/#sec-canonicalizelocalelist and step 3
    // of https://tc39.es/ecma402/#sec-canonicalizeunicodelocaleid.
    // It may be missing other steps.  However, the impl does prove
    // that the Android Java ICU methods can successfully be called.

    ArrayList<String> ret = new ArrayList<String>();
    for (String l : locales) {
      String c = ULocale.getName(l);
      if (!ret.contains(c)) {
        ret.add(c);
      }
    }
    return ret;
  }

  public static String toLocaleLowerCase(List<String> locales, String str)
  {
    return "lowered";
  }

  public static String toLocaleUpperCase(List<String> locales, String str)
  {
    return "uppered";
  }
}
