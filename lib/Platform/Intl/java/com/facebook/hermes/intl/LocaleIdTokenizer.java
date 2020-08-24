package com.facebook.hermes.intl;

public class LocaleIdTokenizer {

    public class LocaleIdSubtagIterationFailed extends Exception {
        public LocaleIdSubtagIterationFailed() {
            super();
        }
    }

    public class LocaleIdSubtag {

        private CharSequence mLocaleIdBuffer = "";
        private int mSubtagStart = 0, mSubtagEnd = 0;

        LocaleIdSubtag(CharSequence localeIdBuffer, int subtagStart, int subtagEnd) {
            mLocaleIdBuffer = localeIdBuffer;
            mSubtagStart = subtagStart;
            mSubtagEnd = subtagEnd;
        }

        public void reset() {
            mLocaleIdBuffer = "";
            mSubtagStart = 0;
            mSubtagEnd = 0;
        }

        public String toString() {
            return mLocaleIdBuffer.subSequence(mSubtagStart, mSubtagEnd + 1).toString();
        }

        public String toLowerString() {

            StringBuffer destination = new StringBuffer();
            for (int idx = mSubtagStart; idx <= mSubtagEnd; idx++) {
                destination.append(Character.toLowerCase(mLocaleIdBuffer.charAt(idx)));
            }

            return destination.toString();
        }

        public String toUpperString() {
            StringBuffer destination = new StringBuffer();
            for (int idx = mSubtagStart; idx <= mSubtagEnd; idx++) {
                destination.append(Character.toUpperCase(mLocaleIdBuffer.charAt(idx)));
            }

            return destination.toString();
        }

        public String toTitleString() {
            StringBuffer destination = new StringBuffer();
            for (int idx = mSubtagStart; idx <= mSubtagEnd; idx++) {
                if (idx == mSubtagStart)
                    destination.append(Character.toUpperCase((mLocaleIdBuffer.charAt(idx))));
                else
                    destination.append(Character.toLowerCase(mLocaleIdBuffer.charAt(idx)));
            }

            return destination.toString();
        }

        public boolean isUnicodeLanguageSubtag() {
            return TextUtils.isUnicodeLanguageSubtag(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }


        public boolean isExtensionSingleton() {
            return TextUtils.isExtensionSingleton(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isUnicodeScriptSubtag() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = alpha{4};
            return TextUtils.isUnicodeScriptSubtag(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isUnicodeRegionSubtag() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            //= (alpha{2} | digit{3}) ;
            return TextUtils.isUnicodeRegionSubtag(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isUnicodeVariantSubtag() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = (alphanum{5,8}
            // | digit alphanum{3}) ;
            return TextUtils.isUnicodeVariantSubtag(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isUnicodeExtensionAttribute() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = (alphanum{3,8}
            return TextUtils.isUnicodeExtensionAttribute(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isUnicodeExtensionKey() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = alphanum alpha ;

            return TextUtils.isUnicodeExtensionKey(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isUnicodeExtensionKeyTypeItem() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = alphanum alpha ;
            return TextUtils.isUnicodeExtensionKeyTypeItem(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isTranformedExtensionTKey() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            //  	= alpha digit ;
            return TextUtils.isTranformedExtensionTKey(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isTranformedExtensionTValueItem() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = (sep alphanum{3,8})+ ;
            return TextUtils.isTranformedExtensionTValueItem(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isPrivateUseExtension() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = (sep alphanum{1,8})+ ;
            return TextUtils.isPrivateUseExtension(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

        public boolean isOtherExtension() {
            // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
            // = (sep alphanum{2,8})+ ;
            return TextUtils.isOtherExtension(mLocaleIdBuffer, mSubtagStart, mSubtagEnd);
        }

    }

    private CharSequence mLocaleIdBuffer;
    int currentSubtagStart = 0, currentSubtagEnd = -1;

    private static boolean isSubtagSeparator(char c) {
        return c == '-' ;
    }

    public LocaleIdTokenizer(CharSequence localeIdBuffer) {
        mLocaleIdBuffer = localeIdBuffer;
    }

    public boolean hasMoreSubtags() {
        return mLocaleIdBuffer.length() > 0 && currentSubtagEnd < mLocaleIdBuffer.length() - 1;
    }

    public LocaleIdSubtag nextSubtag() throws LocaleIdSubtagIterationFailed {

        if (!hasMoreSubtags()) {
            throw new LocaleIdSubtagIterationFailed();
        }

        // Jump over the separator if not the first subtag.
        if (currentSubtagEnd >= currentSubtagStart) {
            if (!isSubtagSeparator(mLocaleIdBuffer.charAt(currentSubtagEnd + 1))) {
                throw new LocaleIdSubtagIterationFailed();
            }

            // If there is not tokens after the separator.
            if (currentSubtagEnd + 2 == mLocaleIdBuffer.length()) {
                throw new LocaleIdSubtagIterationFailed();
            }

            currentSubtagStart = currentSubtagEnd + 2;
        }


        for (currentSubtagEnd = currentSubtagStart; currentSubtagEnd < mLocaleIdBuffer.length() && !isSubtagSeparator(mLocaleIdBuffer.charAt(currentSubtagEnd)); currentSubtagEnd++)
            ;

        if (currentSubtagEnd > currentSubtagStart) {
            currentSubtagEnd--;
            return new LocaleIdSubtag(mLocaleIdBuffer, currentSubtagStart, currentSubtagEnd);
        } else {
            throw new LocaleIdSubtagIterationFailed();
        }
    }
}
