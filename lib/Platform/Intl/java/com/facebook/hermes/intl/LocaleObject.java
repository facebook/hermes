package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Locale;

// A class which is supposed to wrap various represenations of a locale. It is not designed as a representation of the Intl::Locale object as defined by https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Locale/Locale
public class LocaleObject {
    private ULocale icu4jLocale;
    private Locale legacylocale;

    private boolean mSubtagsParsed = false;
    StringBuffer mLanguageSubtagBuffer = null;
    StringBuffer mScriptSubtagBuffer = null;
    StringBuffer mRegionSubtagBuffer = null;
    ArrayList<String> mVariantSubtagList = null;
    StringBuffer mExtensionAndPrivateUseSequenceBuffer = null;


    private LocaleObject(ULocale uLocale) {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        icu4jLocale = uLocale;
    }

    public LocaleObject (StringBuffer languageSubtagBuffer, StringBuffer scriptSubtagBuffer,
                                                         StringBuffer regionSubtagBuffer, ArrayList<String> variantSubtagList,
                                                         StringBuffer extensionAndPrivateUseSequenceBuffer) throws JSRangeErrorException{

        mLanguageSubtagBuffer = new StringBuffer(languageSubtagBuffer);
        mScriptSubtagBuffer = new StringBuffer(scriptSubtagBuffer);
        mRegionSubtagBuffer = new StringBuffer(regionSubtagBuffer);
        mVariantSubtagList = (ArrayList<String>)variantSubtagList.clone();
        mExtensionAndPrivateUseSequenceBuffer = new StringBuffer(extensionAndPrivateUseSequenceBuffer);
        mSubtagsParsed = true;

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            ULocale.Builder localeBuilder = new ULocale.Builder();

            LocaleIdentifier.replaceLanguageSubtagIfNeeded(languageSubtagBuffer, scriptSubtagBuffer, regionSubtagBuffer);
            localeBuilder.setLanguage(languageSubtagBuffer.toString());

            if (scriptSubtagBuffer.length() > 0) {
                localeBuilder.setScript(scriptSubtagBuffer.toString());
            }

            if (regionSubtagBuffer.length() > 0) {
                localeBuilder.setRegion(LocaleIdentifier.replaceRegionSubtagIfNeeded(regionSubtagBuffer));
            }

            if (!variantSubtagList.isEmpty()) {
                localeBuilder.setVariant(TextUtils.join("-", variantSubtagList));
            }

            if (extensionAndPrivateUseSequenceBuffer.length() > 0) {

                if (extensionAndPrivateUseSequenceBuffer.charAt(extensionAndPrivateUseSequenceBuffer.length() - 1) == '-') {
                    throw new JSRangeErrorException("Incomplete singleton");
                }

                String extensions[] = new String[26];
                LocaleIdentifier.parseExtensionSequence(extensionAndPrivateUseSequenceBuffer, extensions);

                for (int i = 0; i < 26; i++) {
                    if (extensions[i] != null && !extensions[i].isEmpty())
                        localeBuilder.setExtension((char) ('a' + i), extensions[i]);
                }
            }

            icu4jLocale = localeBuilder.build();

        } else {

            StringBuffer localeIdBuffer = new StringBuffer();

            localeIdBuffer.append(languageSubtagBuffer);

            if (scriptSubtagBuffer.length() > 0) {
                localeIdBuffer.append('-');
                localeIdBuffer.append(scriptSubtagBuffer);
            }

            if (regionSubtagBuffer.length() > 0) {
                localeIdBuffer.append('-');
                localeIdBuffer.append(regionSubtagBuffer);
            }

            if (!variantSubtagList.isEmpty()) {
                localeIdBuffer.append('-');
                localeIdBuffer.append(TextUtils.join("-", variantSubtagList));
            }

            if (extensionAndPrivateUseSequenceBuffer.length() > 0) {

                if (extensionAndPrivateUseSequenceBuffer.charAt(extensionAndPrivateUseSequenceBuffer.length() - 1) == '-') {
                    throw new JSRangeErrorException("Incomplete singleton");
                }

                String extensions[] = new String[26];
                LocaleIdentifier.parseExtensionSequence(extensionAndPrivateUseSequenceBuffer, extensions);

                for (int i = 0; i < 26; i++) {
                    if (extensions[i] != null && !extensions[i].isEmpty()) {
                        localeIdBuffer.append('-');
                        localeIdBuffer.append((char) ('a' + i));
                        localeIdBuffer.append('-');
                        localeIdBuffer.append(extensions[i]);
                    }
                }
            }

            legacylocale = Locale.forLanguageTag(localeIdBuffer.toString());
        }
    }

    private LocaleObject(Locale locale) {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        legacylocale = locale;
    }

    private static void copyLocaleStringToBufferNormalized(StringBuffer buffer, String locale) throws JSRangeErrorException{
        assert(buffer.length() == 0);

        // Normalize the string by
        // 1. lower casing
        // 2. Convert '_' to - [Nope: We are not doing it as test262 has tests which validates that the separator is hyphen]
        // 3. avoiding leading and trailing whitespaces [Nope; We are not doing it as test262 has tests which validates that the leading and trailing spaces should throw]

        // Seek past initial spaces.
        // int idx=0;
        // while (locale.charAt(idx) == ' ')idx++;
        if(locale.charAt(0) == ' ' || locale.charAt(locale.length()-1) == ' ')
            throw new JSRangeErrorException("Incorrect locale information provided");


        int idx=0;
        for(; idx<locale.length(); idx++) {
            char localeChar = locale.charAt(idx);

            //if(localeChar == '_') {
            //    buffer.append('-');
            //    continue;
            //}

            buffer.append(Character.toLowerCase(localeChar));
        }

        // idx=buffer.length()-1;
        // while (buffer.charAt(idx) == ' ') idx--;
        // buffer.delete(idx+1, buffer.length());
    }

    public static LocaleObject constructFromLocaleId(String localeId) throws JSRangeErrorException {
        StringBuffer languageSubtag = new StringBuffer(8);
        StringBuffer scriptSubtag = new StringBuffer(4);
        StringBuffer regionSubtag = new StringBuffer(4);
        ArrayList<String> variantSubtagList = new ArrayList<>();
        StringBuffer extensionAndPrivateUseSequence = new StringBuffer();

        StringBuffer localeBuffer = new StringBuffer();
        copyLocaleStringToBufferNormalized(localeBuffer, localeId);

        // TODO :: Consolidate throws ..
        if(!LocaleIdentifier.canonicalizeLocaleIdIntoParts(localeBuffer, languageSubtag, scriptSubtag, regionSubtag, variantSubtagList, extensionAndPrivateUseSequence)) {
            throw new JSRangeErrorException(String.format("Incorrect locale information provided: %s", localeId==null? "null":localeId));
        }

        return LocaleObject.constructFromSubtags(languageSubtag, scriptSubtag, regionSubtag, variantSubtagList, extensionAndPrivateUseSequence);
    }

    public static LocaleObject constructFromICU4jLocale(ULocale uLocale) {
        return new LocaleObject(uLocale);
    }

    public static LocaleObject constructFromLegacyLocale(Locale locale) {
        return new LocaleObject(locale);
    }

    public static LocaleObject constructFromSubtags(StringBuffer languageSubtagBuffer, StringBuffer scriptSubtagBuffer,
                                                         StringBuffer regionSubtagBuffer, ArrayList<String> variantSubtagList,
                                                         StringBuffer extensionAndPrivateUseSequenceBuffer) throws JSRangeErrorException{

        return new LocaleObject(languageSubtagBuffer, scriptSubtagBuffer, regionSubtagBuffer, variantSubtagList, extensionAndPrivateUseSequenceBuffer);
    }

    // Construct a LocaleObject corresponding to the current "default" locale for the user ..
    public static LocaleObject constructDefault() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return new LocaleObject(ULocale.getDefault(ULocale.Category.FORMAT));
        } else {
            return new LocaleObject(Locale.getDefault(Locale.Category.FORMAT));
        }
    }

    public String toLocaleId() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return icu4jLocale.toLanguageTag();
        } else {
            return legacylocale.toLanguageTag();
        }
    }

    public boolean matches(LocaleObject candidate) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return icu4jLocale.compareTo(candidate.icu4jLocale) == 0;
        } else {
            throw new UnsupportedOperationException("We don't yet support matching legacy locales.");
        }
    }

    public ULocale getICU4jLocale() {
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.N) {
            throw new UnsupportedOperationException("ICU4j locale object not available !");
        }

        assert(icu4jLocale != null);
        return icu4jLocale;
    }

    public Locale getLegacyLocale() {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.N) {
            throw new UnsupportedOperationException("ICU4j locale should be used instead of legacy Localy !");
        }

        assert(legacylocale != null);
        return legacylocale;
    }
}
