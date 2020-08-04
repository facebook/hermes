package com.facebook.hermes.intl;

public class Constants {
    public static final String LOCALEMATCHER = "localeMatcher";
    public static final String LOCALEMATCHER_BESTFIT = "best fit";
    public static final String LOCALEMATCHER_LOOKUP = "lookup";
    public static final String[] LOCALEMATCHER_POSSIBLE_VALUES = {LOCALEMATCHER_BESTFIT, LOCALEMATCHER_LOOKUP};

    public static final String SENSITIVITY = "sensitivity";
    public static final String SENSITIVITY_BASE = "base";
    public static final String SENSITIVITY_ACCENT = "accent";
    public static final String SENSITIVITY_CASE = "case";
    public static final String SENSITIVITY_VARIANT = "varant";
    public static final String[] SENSITIVITY_POSSIBLE_VALUES = {SENSITIVITY_BASE, SENSITIVITY_ACCENT, SENSITIVITY_CASE, SENSITIVITY_VARIANT};

    public static final String IGNOREPUNCTUATION = "ignorePunctuation";
    public static final String NUMERIC = "numeric";
    public static final String CASEFIRST = "caseFirst";

    public static final String SORT = "sort";
    public static final String SEARCH = "search";

    public static final String USAGE = "usage";
    public static final String[] COLLATOR_USAGE_POSSIBLE_VALUES = {SORT, SEARCH};
}
