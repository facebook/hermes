/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents the Java part of the Android
 * Intl.DateTimeFormat implementation.  The interaction with the
 * Hermes JaveScript internals are implemented in C++ and should not
 * generally need to be changed.  Implementers' notes here will
 * describe what parts of the ECMA 402 spec remain to be implemented.
 *
 * Implementer notes:
 *
 * Internal slots: In the ECMA 402 spec, there are a number of
 * references to internal slots.  These are gneerally expressed in
 * terms of JavaScript objects, but the semantics do not generally
 * depend on this.  For example, where the spec says
 * "Intl.DateTimeFormat instances have an
 * [[InitializedDateTimeFormat]] internal slot", this would not be a
 * literal artifact of the implementation.  Internal slots, where
 * necessary, should be represented as members of this java
 * DateTimeFormat object.
 *
 * ICU4J vs Unicode: The ECMA 402 spec makes reference to Unicode
 * documents and data, such as Unicode Technical Standard 35 and the
 * Common Locale Data Repository.  However, in practice, platform
 * Unicode implementations are based on the ICU libraries, which
 * encapsulate the concepts in the Unicode documents and data, and do
 * not provide direct access.  The Android Intl platform code is
 * expected to be implemented in terms of the Android ICU libraries
 * such as android.icu.text
 * <https://developer.android.com/reference/android/icu/text/package-summary>
 * and android.icu.number
 * <https://developer.android.com/reference/android/icu/number/package-summary>,
 * which are themselves derived from the icu4j API
 * <https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/>.
 * This strategy allows for a more space-efficient implementation than
 * one which ships its own copy of the locale data, which can be 20mb
 * or more.  That said, it can be difficult to understand precisely
 * how to implement ECMA 402 in terms of icu4j, and I've left much of
 * the specifics of this up to the implementer.  Comparison with
 * existing implementations may help.
 */
public class DateTimeFormat {
  // options are localeMatcher:string, calendar:string, numberingSystem:string, hour12:boolean,
  // hourCycle:string, timeZone:string, formatMatcher:string, weekday:string, era:string,
  // year:string, month:string, day:string, hour:string, minute:string, second:string,
  // timeZoneName:string
  //
  // Implementer note: The ctor corresponds roughly to
  // https://tc39.es/ecma402/#sec-initializedatetimeformat.
  //
  // Locales and Options: But, some of the steps described in the
  // algorithm in the spec have already been performed.  Most of
  // CanonicalizeLocaleList has already been done; only steps
  // 7.c.v-vii remain to be done for each element in the input locales
  // list.  The options argument will only contain the correct option
  // strings, whose names have already been converted to appropriate
  // Java types (Boolean, Number, or String).  Where the spec dictates
  // a set of possible values, that check has not yet been done.  It
  // is the responsibility of the ctor to store whatever locale and
  // option data is needed to implement the other members of this
  // class.
  public DateTimeFormat(List<String> locales, Map<String, Object> options)
    throws JSRangeErrorException
  {}

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof.
  //
  // The notes on the ctor for Locales and Options also apply here.
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) {
    return locales;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.datetimeformat.prototype.resolvedoptions
  //
  // Steps 1-4 are implied; all that is necessary is to perform step
  // 5, except that CreateDataPropertyOrThrow needs to be mapped into
  // Java.  Instead, a Java Map must be returned, where keys are the
  // expected property names, and values are Boolean, Number, or
  // String.  Note that the types are implied when the "internal
  // slots" are set (in the ctor), but in practice each "slot" should
  // correspond to a member with a well-defined Java type.
  public Map<String, Object> resolvedOptions() {
    return new HashMap<String, Object>();
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetime
  //
  // Steps 1 and 2 of PartitionDateTimePattern are already done.  The
  // formal steps require construction of several internal
  // NumberFormat JavaScript objects, but these objects are never
  // exposed; it should be possible to create and use java
  // NumberFormat objects only.
  public String format(double jsTimeValue) {
    return (new SimpleDateFormat()).format(new Date((long) jsTimeValue));
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatdatetimetoparts
  public List<Map<String, String>> formatToParts(double jsTimeValue) {
    ArrayList<Map<String, String>> ret = new ArrayList<Map<String, String>>();
    HashMap<String, String> part = new HashMap<String, String>();
    part.put("month", "integer");
    // This isn't right, but I didn't want to do more work for a stub.
    part.put("value", (new SimpleDateFormat()).format(new Date((long) jsTimeValue)));
    ret.add(part);
    return ret;
  }
}
