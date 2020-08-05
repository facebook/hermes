package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

// A class which is supposed to wrap various represenations of a "locale".
// It is not designed as a representation of the Intl::Locale object as defined by https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Locale/Locale
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

    private static boolean isAlphaNum(String name, int min, int max) {

        int length = name.length();
        if (length < min || length > max) {
            return false;
        }

        for (int idx=0; idx<name.length(); idx++) {
            char c = name.charAt(idx);
            if (!Character.isLetter(c) && !Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isUnicodeExtensionAttribute(String name) {
        return isAlphaNum(name, 3,8);
    }

    // key = alphanum alpha;
    private static boolean isUnicodeExtensionkeywordKey(String name) {
        return name.length() == 2 && (Character.isLetter(name.charAt(0)) || Character.isDigit(name.charAt(0))) && Character.isLetter(name.charAt(1));
    }

    // type = alphanum{3,8}
    private static boolean isUnicodeExtensionkeywordType(String name) {
        return isAlphaNum(name, 3, 8);
    }

    public void getUnicodeExtensions(ArrayList<String> attributes, HashMap<String, String> keywords, HashMap<String, String> defaults) throws JSRangeErrorException {
        assert (mSubtagsParsed); // TODO Need to ensure it.

        assert (attributes != null);
        assert (keywords != null);

        if (mExtensionAndPrivateUseSequenceBuffer == null || mExtensionAndPrivateUseSequenceBuffer.length() == 0) {
            return;
        }

        String extensions[] = new String[26];
        LocaleIdentifier.parseExtensionSequence(mExtensionAndPrivateUseSequenceBuffer, extensions);
        String unicodeExtensions = extensions['u' - 'a'];
        if (unicodeExtensions == null)
            return;

        // TODO:: Relatively unoptimized implementation ..
        String[] tokens = unicodeExtensions.split("-");

        int tokenIndex = 0;
        // find all the attributes which are guaranteed to be at the start of canonicalized tags.
        while (tokenIndex < tokens.length) {
            String token = tokens[tokenIndex];
            if (isUnicodeExtensionAttribute(token)) {
                attributes.add(token);
                tokenIndex++;
            } else {
                break;
            }
        }

        while (tokenIndex < tokens.length) {
            String token = tokens[tokenIndex];

            String key = null, type = null;

            if (isUnicodeExtensionkeywordKey(token)) {
                key = token;
            } else {
                throw new JSRangeErrorException("Unicode extension keyword key expected !");
            }

            ++tokenIndex;
            if (tokenIndex >= tokens.length) {
                if (defaults.containsKey(key)) {
                    keywords.put(key, defaults.get(key));
                }
                break;
            }

            token = tokens[tokenIndex];
            if (isUnicodeExtensionkeywordKey(token)) {
                if (defaults.containsKey(key)) {
                    keywords.put(key, defaults.get(key));
                }
                continue;
            }

            if (isUnicodeExtensionkeywordType(token)) {
                type = token;
            } else {
                throw new JSRangeErrorException("Unicode extension keyword type expected !");
            }
            tokenIndex++;

            keywords.put(key, type);
        }
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

    public static LocaleObject constructFromLocaleId(String localeId, boolean stripExtensions) throws JSRangeErrorException {
        StringBuffer languageSubtag = new StringBuffer(8);
        StringBuffer scriptSubtag = new StringBuffer(4);
        StringBuffer regionSubtag = new StringBuffer(4);
        ArrayList<String> variantSubtagList = new ArrayList<>();
        StringBuffer extensionAndPrivateUseSequence = new StringBuffer();

        StringBuffer localeBuffer = new StringBuffer();
        copyLocaleStringToBufferNormalized(localeBuffer, localeId);

        if(!LocaleIdentifier.canonicalizeLocaleIdIntoParts(localeBuffer, languageSubtag, scriptSubtag, regionSubtag, variantSubtagList, extensionAndPrivateUseSequence)) {
            throw new JSRangeErrorException(String.format("Incorrect locale information provided: %s", localeId==null? "null":localeId));
        }

        return LocaleObject.constructFromSubtags(languageSubtag, scriptSubtag, regionSubtag, variantSubtagList, stripExtensions ? new StringBuffer() : extensionAndPrivateUseSequence);
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

    public static LocaleObject constructByExtendingExistingObject(LocaleObject baseLocaleObject, char extensionSingleton, String extension) {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            // Put the collation extension keys back if needed.
            ULocale.Builder builder = new ULocale.Builder();
            builder.setLocale(baseLocaleObject.getICU4jLocale());

            if(extension.length() > 0) {
                builder.setExtension(extensionSingleton, extension);
            }

            return LocaleObject.constructFromICU4jLocale(builder.build());
        } else {
            throw new UnsupportedOperationException("Extending an existing locale object is not supported !");
        }
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

    private void ensureSubtagsAvailable() {
        if(!mSubtagsParsed) {

        }
    }

    public boolean matches(LocaleObject candidate) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N && icu4jLocale.compareTo(candidate.icu4jLocale) == 0) {
            return true;
        }

        return false;
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
