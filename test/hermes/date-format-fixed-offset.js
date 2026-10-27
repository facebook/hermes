/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ="EST5EDT,M3.2.0,M11.1.0" %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: TZ="AEST-10AEDT,M10.1.0,M4.1.0/3" %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: TZ="America/New_York" %hermes -O %s | %FileCheck --match-full-lines --check-prefix=NEAR %s
// RUN: TZ="EST5EDT,M3.2.0,M11.1.0" %shermes -exec %s | %FileCheck --match-full-lines %s
// UNSUPPORTED: locale_aware_dates || intl || windows
"use strict";

// The fixed toLocale*String formatter must use the same UTC offset as the Date
// getters, including for dates before 1970 and after 2037. With a 64-bit
// time_t, both pass every date to the OS unchanged, so they agree in any
// timezone. (glibc applies POSIX rule strings only from 1970 on; it gives
// earlier dates one fixed offset.) America/New_York adds historical offsets
// where the IANA timezone database is installed (without it, the zone falls
// back to UTC and still agrees). With a 32-bit time_t, dates beyond 1901-2038
// go through two different equivalent-year mappings. They agree on the noon
// dates below under the two POSIX rules, but not under every rule or zone, so
// the America/New_York run checks only the dates inside that range.

var months = [
  'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
  'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec',
];

function pad(n) {
  return n < 10 ? '0' + n : '' + n;
}

/// Format the date of \p d like the fixed formatter, from the Date getters.
function dateFromGetters(d) {
  return months[d.getMonth()] + ' ' + d.getDate() + ', ' + d.getFullYear();
}

/// Format the time of \p d like the fixed formatter, from the Date getters.
function timeFromGetters(d) {
  var h = d.getHours();
  return (h % 12 || 12) + ':' + pad(d.getMinutes()) + ':' +
      pad(d.getSeconds()) + (h < 12 ? ' AM' : ' PM');
}

/// \return noon on the 15th and 25th of every month of each year in
/// \p years. Those days are never DST transition days for the POSIX rules
/// above.
function middleOfMonths(years) {
  var dates = [];
  for (var i = 0; i < years.length; ++i) {
    for (var m = 0; m < 12; ++m) {
      dates.push(new Date(years[i], m, 15, 12), new Date(years[i], m, 25, 12));
    }
  }
  return dates;
}

/// Print each date in \p dates whose toLocale*String output disagrees with its
/// getters, then the number of such dates.
function check(dates) {
  var mismatches = 0;
  for (var i = 0; i < dates.length; ++i) {
    var d = dates[i];
    var date = dateFromGetters(d);
    var time = timeFromGetters(d);
    if (d.toLocaleString() !== date + ', ' + time ||
        d.toLocaleDateString() !== date || d.toLocaleTimeString() !== time) {
      print(d.toISOString() + ': ' + d.toLocaleString() + ' != ' + date +
          ', ' + time);
      ++mismatches;
    }
  }
  print('mismatches: ' + mismatches + ' of ' + dates.length);
}

print('near');
// CHECK-LABEL: near
// NEAR-LABEL: near
check(middleOfMonths(
    [1902, 1918, 1943, 1945, 1960, 1966, 1969, 1970, 2007, 2037]));
// CHECK-NEXT: mismatches: 0 of 240
// NEAR-NEXT: mismatches: 0 of 240

print('far');
// CHECK-LABEL: far
var far = middleOfMonths([-1000, 1850, 1900, 2040, 2100, 275000]);
far.push(new Date(-8.64e15), new Date(8.64e15));
check(far);
// CHECK-NEXT: mismatches: 0 of 146
