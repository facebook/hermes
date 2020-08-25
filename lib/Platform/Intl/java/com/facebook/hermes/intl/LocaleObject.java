package com.facebook.hermes.intl;

import android.os.Build;

public class LocaleObject {
    public static ILocaleObject createDefault() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
            return LocaleObjectICU4J.createDefault();
        else
            return LocaleObjectAndroid.createDefault();
    }

    public static ILocaleObject createFromLocaleId(String localeId) throws JSRangeErrorException {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
            return LocaleObjectICU4J.createFromLocaleId(localeId);
        else
            return LocaleObjectAndroid.createFromLocaleId(localeId);
    }
}
