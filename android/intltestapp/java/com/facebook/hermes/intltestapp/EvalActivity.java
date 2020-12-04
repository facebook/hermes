package com.facebook.hermes.intltestapp;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class EvalActivity extends Activity {

    static {
        System.loadLibrary("hermes");
        System.loadLibrary("hermesevalapp");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_eval);

        String result = nativeEvalScript("var i=new Intl.Collator().compare('ab', 'ab'); i;");
        TextView responseTextView = (TextView)this.findViewById(R.id.responseTextView);
        responseTextView.setText(result);
    }

    static native String nativeEvalScript(String s);
}