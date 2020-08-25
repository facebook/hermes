package com.facebook.hermes.intl;

public interface IPlatformCollator {
    int compare(String source, String target);

    boolean isSensitiySupported(String sensitivity);
    void setSensitivity(String sensitivity);

    boolean isIgnorePunctuationSupported();
    void setIgnorePunctuation(boolean ignore);

    boolean isNumericCollationSupported();
    void setNumericAttribute(boolean numeric);

    boolean isCaseFirstCollationSupported();
    void setCaseFirstAttribute(String caseFirst);
}
