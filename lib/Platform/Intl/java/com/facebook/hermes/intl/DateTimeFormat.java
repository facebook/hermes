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

public class DateTimeFormat {
  // options are localeMatcher:string, calendar:string, numberingSystem:string, hour12:boolean,
  // hourCycle:string, timeZone:string, formatMatcher:string, weekday:string, era:string,
  // year:string, month:string, day:string, hour:string, minute:string, second:string,
  // timeZoneName:string
  public DateTimeFormat(List<String> locales, Map<String, Object> options) {
  }

  // options are localeMatcher:string
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) {
    return locales;
  }

  // https://tc39.es/ecma402/#sec-intl.datetimeformat.prototype.resolvedoptions
  public Map<String, Object> resolvedOptions() {
    return new HashMap<String, Object>();
  }

  public String format(double jsTimeValue) {
    return (new SimpleDateFormat()).format(new Date((long) jsTimeValue));
  }

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
