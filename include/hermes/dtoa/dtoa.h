/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// dtoa doesn't provide a header file so this simple one was created.

#ifndef HERMES_DTOA_DTOA_H
#define HERMES_DTOA_DTOA_H

#ifdef __cplusplus
extern "C" {
#endif

/// Converts double into ascii string.
/// \param dd the double to convert.
/// \param mode the rounding mode, 0 for default.<ul>
///     <li>0 ==> shortest string that yields d when read in
///             and rounded to nearest.
///     <li>1 ==> like 0, but with Steele & White stopping rule;
///             e.g. with IEEE P754 arithmetic , mode 0 gives
///             1e23 whereas mode 1 gives 9.999999999999999e22.
///     <li>2 ==> max(1,ndigits) significant digits.  This gives a
///             return value similar to that of ecvt, except
///             that trailing zeros are suppressed.
///     <li>3 ==> through ndigits past the decimal point.  This
///             gives a return value similar to that from fcvt,
///             except that trailing zeros are suppressed, and
///             ndigits can be negative.
///     <li>4,5 ==> similar to 2 and 3, respectively, but (in
///             round-nearest mode) with the tests of mode 0 to
///             possibly return a shorter string that rounds to d.
///             With IEEE arithmetic and compilation with
///             -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
///             as modes 2 and 3 when FLT_ROUNDS != 1.
///     <li>6-9 ==> Debugging modes similar to mode - 4:  don't try
///             fast floating-point estimate (if applicable).
///     <li>Values of mode other than 0-9 are treated as mode 0.
/// </ul>
/// \param ndigits number of digits of precision, 0 for default.
/// \param decpt where to store position of the decimal. (n in ES5.1 9.8.1)
/// \param sign location to store 1 if negative number, 0 if positive number.
/// \param rve location to store pointer to the end of the returned string.
/// \return string representation of s in ES5.1 9.8.1
char *
g_dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);

/// Same as dtoa, but #defines ROUND_BIASED, which enables the mode which is
/// used for getting results with a fixed number of digits after the decimal.
/// It also modifies a check in dtoa (see the NOTE in dtoa_fixed.c),
/// which ensures that 0.5 does not get flushed to 0, but rather rounds up to 1.
/// A separate function is necessary because dtoa needs compilation flags
/// to change options and provides no runtime means of doing so,
/// and modification of the code was needed to ensure correctly biased rounding.
char *dtoa_fixedpoint(
    double dd,
    int mode,
    int ndigits,
    int *decpt,
    int *sign,
    char **rve);

void g_freedtoa(char *);

char *g_fmt(char *, double);
double hermes_g_strtod(const char *s00, char **se);

#ifdef __cplusplus
}
#endif

#endif // HERMES_DTOA_DTOA_H
