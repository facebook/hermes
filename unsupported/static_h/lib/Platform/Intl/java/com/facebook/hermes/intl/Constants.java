/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

public class Constants {
  public static final String LOCALE = "locale";

  public static final String LOCALEMATCHER = "localeMatcher";
  public static final String LOCALEMATCHER_BESTFIT = "best fit";
  public static final String LOCALEMATCHER_LOOKUP = "lookup";
  public static final String[] LOCALEMATCHER_POSSIBLE_VALUES = {
    LOCALEMATCHER_BESTFIT, LOCALEMATCHER_LOOKUP
  };

  public static final String COLLATION = "collation";
  public static final String COLLATION_DEFAULT = "default";
  public static final String COLLATION_SEARCH = "search";
  public static final String COLLATION_STANDARD = "standard";
  public static final String COLLATION_INVALID = "invalid";
  public static final String[] COLLATION_OVERRIDE_TO_DEFAULT_VALUES = {
    COLLATION_SEARCH, COLLATION_STANDARD, COLLATION_INVALID
  };

  // Ref: http://cldr.unicode.org/core-spec
  public static final String COLLATION_EXTENSION_KEY_SHORT = "co";
  public static final String COLLATION_EXTENSION_KEY_LONG = "collation";

  public static final String COLLATION_EXTENSION_PARAM_NUMERIC_SHORT = "kn";
  public static final String COLLATION_EXTENSION_PARAM_NUMERIC_LONG = "colNumeric";

  public static final String COLLATION_EXTENSION_PARAM_CASEFIRST_SHORT = "kf";
  public static final String COLLATION_EXTENSION_PARAM_CASEFIRST_LONG = "colCaseFirst";

  public static final String COLLATION_OPTION_SENSITIVITY = "sensitivity";
  public static final String SENSITIVITY_BASE = "base";
  public static final String SENSITIVITY_ACCENT = "accent";
  public static final String SENSITIVITY_CASE = "case";
  public static final String SENSITIVITY_VARIANT = "variant";
  public static final String[] SENSITIVITY_POSSIBLE_VALUES = {
    SENSITIVITY_BASE, SENSITIVITY_ACCENT, SENSITIVITY_CASE, SENSITIVITY_VARIANT
  };

  public static final String COLLATION_OPTION_IGNOREPUNCTUATION = "ignorePunctuation";
  public static final String COLLATION_OPTION_NUMERIC = "numeric";
  public static final String COLLATION_OPTION_CASEFIRST = "caseFirst";

  public static final String CASEFIRST_UPPER = "upper";
  public static final String CASEFIRST_LOWER = "lower";
  public static final String CASEFIRST_FALSE = "false";
  public static final String[] CASEFIRST_POSSIBLE_VALUES = {
    CASEFIRST_UPPER, CASEFIRST_LOWER, CASEFIRST_FALSE
  };

  public static final String SORT = "sort";
  public static final String SEARCH = "search";

  public static final String COLLATION_OPTION_USAGE = "usage";
  public static final String[] COLLATOR_USAGE_POSSIBLE_VALUES = {SORT, SEARCH};
}
