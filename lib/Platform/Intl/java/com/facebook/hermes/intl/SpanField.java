package com.facebook.hermes.intl;

import androidx.annotation.NonNull;

import java.io.InvalidObjectException;
import java.text.DateFormat;

/**
 * Backport of <a href="https://android.googlesource.com/platform/external/icu/+/refs/heads/main/android_icu4j/src/main/java/android/icu/text/DateIntervalFormat.java#350">DateIntervalFormat.SpanField</a>
 * which is private.
 */
public final class SpanField extends DateFormat.Field {
    /**
     * The concrete field used for spans in formatRangeToParts.
     *
     * Instances of DATE_INTERVAL_SPAN have an associated value.
     *
     * If 0, the date fields within the span are for the "from" date.
     * If 1, the date fields within the span are for the "to" date.
     * If 2, the literals are "shared".
     */
    public static final SpanField DATE_INTERVAL_SPAN = new SpanField("date-interval-span");

    private SpanField(String name) {
        super(name, -1);
    }

    @NonNull
    @Override
    protected Object readResolve() throws InvalidObjectException {
        if (this.getName().equals(DATE_INTERVAL_SPAN.getName()))
            return DATE_INTERVAL_SPAN;
        throw new InvalidObjectException("An invalid object.");
    }
}
