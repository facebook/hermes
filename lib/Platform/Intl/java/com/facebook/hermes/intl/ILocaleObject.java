package com.facebook.hermes.intl;

import java.util.ArrayList;

public interface ILocaleObject <T> {
    ArrayList<String> getUnicodeExtensions(String key) throws JSRangeErrorException;
    ArrayList<String> getVariants() throws JSRangeErrorException;

    void setVariant(ArrayList<String> newVariants) throws JSRangeErrorException;

    void setUnicodeExtensions(String key, ArrayList<String> type) throws JSRangeErrorException;

    T getLocale() throws JSRangeErrorException;
    T getLocaleWithoutExtensions() throws JSRangeErrorException;

    String toCanonicalTag() throws JSRangeErrorException;
    String toCanonicalTagWithoutExtensions() throws JSRangeErrorException;
}
