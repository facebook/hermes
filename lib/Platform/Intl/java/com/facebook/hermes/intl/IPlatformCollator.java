package com.facebook.hermes.intl;

public interface IPlatformCollator {

    int compare(String source, String target);

    boolean isSensitiySupported(String sensitivity);
    void setSensitivity(String sensitivity);

    void setIgnorePunctuation(boolean ignore);
    boolean isIgnorePunctuationSupported();

    boolean isNumericCollationSupported();
    void setNumericAttribute(boolean numeric);

    boolean isCaseFirstCollationSupported();
    public void setCaseFirstAttribute(String caseFirst);
}
