package com.facebook.hermes.replapp;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.Html;
import android.util.Pair;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.facebook.jni.HybridData;
import com.facebook.proguard.annotations.DoNotStrip;
import com.facebook.soloader.SoLoader;
import com.google.gson.Gson;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

public class REPLActivity extends Activity {
    
    // Please not that there are a lots of potentially improvement to the code organization and design here. 
    // I've left it this way considering that this is a test tool.
    // But any improvement would be nice !

    private ListView mScriptListView;
    private EditText mScriptEditText;
    private CustomAdapter mAdapter;

    private ArrayList<String> mScriptStack = new ArrayList<>();
    private int mScriptStackIndex = 0;

    @DoNotStrip
    private HybridData mHybridData;

    private static REPLActivity sActivity = null; // UGLY !

    static {
        System.loadLibrary("hermesreplapp");
    }

    class CustomAdapter extends BaseAdapter {
        private ArrayList<Pair<String, String>> scriptsAndResponses;

        private Context context;

        public CustomAdapter(Context context) {
            this.scriptsAndResponses = new ArrayList<>();
            this.context = context;
        }

        public void add(String script, String response) {
            scriptsAndResponses.add(new Pair<String, String>(script, response));
            this.notifyDataSetChanged();
        }

        public void addEvent(String eventText) {
            scriptsAndResponses.add(new Pair<String, String>(eventText, ""));
            this.notifyDataSetChanged();
        }

        public void addError(String eventText) {
            scriptsAndResponses.add(new Pair<String, String>("", eventText));
            this.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return scriptsAndResponses.size();
        }

        @Override
        public Object getItem(int i) {
            return scriptsAndResponses.get(i);
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int i, View view, ViewGroup viewGroup) {
            Pair<String, String> scriptAndResponse = (Pair<String, String>) getItem(i);

            if(scriptAndResponse.second.isEmpty()) { // Hacky .. we treat this case as event.
                view = LayoutInflater.from(context).inflate(R.layout.listview_event, null);
                TextView eventsListView = view.findViewById(R.id.eventTextView);
                eventsListView.setText(Html.fromHtml(scriptAndResponse.first));
            } else if (scriptAndResponse.first.isEmpty()) { // Hacky .. we treat this case as error.
                view = LayoutInflater.from(context).inflate(R.layout.listview_error, null);
                TextView errorListView = view.findViewById(R.id.errorTextView);
                errorListView.setText(Html.fromHtml(scriptAndResponse.second));
            }
            else {
                view = LayoutInflater.from(context).inflate(R.layout.listview_script, null);
                TextView scriptsListView = view.findViewById(R.id.scriptTextView);
                TextView responseListView = view.findViewById(R.id.responseTextView);

                scriptsListView.setText(Html.fromHtml(scriptAndResponse.first));
                responseListView.setText(Html.fromHtml(scriptAndResponse.second));
            }

            return view;
        }
    }

    private void setScriptEditText(EditText scriptEditText, String text) {
        scriptEditText.setText(text);
        scriptEditText.setSelection(text.length());
    }

    private void popScriptStackUp(EditText scriptEditText) {
        if(mScriptStack.isEmpty())
            return;

        if(mScriptStackIndex == 0)
            return;

        // We are not in stack .. above it ..
        if(mScriptStackIndex == mScriptStack.size()) {
            mScriptStackIndex--;
            setScriptEditText(scriptEditText, mScriptStack.get(mScriptStackIndex));
            return;
        }

        mScriptStackIndex--;
        setScriptEditText(scriptEditText, mScriptStack.get(mScriptStackIndex));
    }

    private void popScriptStackDown(EditText scriptEditText) {
        if(mScriptStack.isEmpty())
            return;

        // We are live .. don't update anything.
        if(mScriptStackIndex == mScriptStack.size()) {
            return;
        }

        // On top of stack .. go live.
        if(mScriptStackIndex == mScriptStack.size() - 1) {
            mScriptStackIndex++;
            scriptEditText.setText("");
            return;
        }

        mScriptStackIndex++;
        setScriptEditText(scriptEditText, mScriptStack.get(mScriptStackIndex));
    }

    private void evalScript(EditText scriptEditText, ListView scriptListView, CustomAdapter adapter) {
        String script = scriptEditText.getText().toString();
    }

    private void runHermesCommand(String command, String[] args) {
        switch (command) {
            case "collect": {
                nativeCollect(args.length > 0 ? args[0] : "User");
                break;
            }
            case "heapstats": {
                String stats = nativeHeapStats();
                mAdapter.addEvent(stats);
                break;
            }
            default:
                mAdapter.addError("Unknown hermes command: " + command);
        }
    }

    private void runDroidCommand(String command, String[] args) {
        switch (command) {
            case "collect": {
                System.gc();
                mAdapter.addEvent("System.gc() triggered.");
                break;
            }
            default:
                mAdapter.addError("Unknown droid command: " + command);
        }
    }

    private void runCommand(String prefix, String command){

        // We have a very simple scheme.. First part is command, rest are args.
        String[] commandParts = command.split("\\s+");

        String commandVerb = commandParts[0];
        String[] commandArgs = Arrays.copyOfRange(commandParts, 1, commandParts.length);

        if(prefix.equals("hm"))
            runHermesCommand(commandVerb, commandArgs);
        else if(prefix.equals("dr"))
            runDroidCommand(commandVerb, commandArgs);
        else
            mAdapter.addError("Unknown prefix: " + prefix);
    }

    // Eg. commands: "hm:collect()"
    private void run() {
        String cmd = mScriptEditText.getText().toString();
        if(cmd.startsWith("hm:")) {
            runCommand("hm", cmd.substring("hm:".length()));
        } else if(cmd.startsWith("dr:")) {
            runCommand("dr", cmd.substring("hm:".length()));
        } else {
            String script = cmd;
            String result = nativeEvalScript(script);
            mAdapter.add(script, result);

            mScriptListView.smoothScrollToPosition(mAdapter.getCount());
            mScriptEditText.setText("");

            mScriptStack.add(script);
            mScriptStackIndex = mScriptStack.size();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_repl);

        SoLoader.init(this, false);
        mHybridData = initHybrid();

        mAdapter = new CustomAdapter(this);

        mScriptListView = (ListView) findViewById(R.id.scriptsListView);
        mScriptListView.setAdapter(mAdapter);

        mScriptEditText = (EditText) findViewById(R.id.scriptEditText);
        mScriptEditText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int actionId, KeyEvent keyEvent) {
                boolean handled = false;
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    run();
                    handled = true;
                }
                return handled;

            }
        });

        // Alternatively use adb to input text
        // adb shell input text "new\ Intl\.Collator\(\"de\"\,\ \{usage\:\ \"sort\"\}\)"

        // adb shell am broadcast -a com.facebook.hermes.intltest.eval --es "script" "var\ x=\'abcd\'\;x;"
        // adb shell am broadcast -a com.facebook.hermes.intltest.eval --es "script" "new\ Intl.Collator();"
        this.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String script = intent.getStringExtra("script");
                mScriptEditText.setText(script);
                run();
            }
        }, new IntentFilter("com.facebook.hermes.intltest.eval"));

        Button evalButton = (Button) findViewById(R.id.evalButton);
        evalButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String script = mScriptEditText.getText().toString();
                run();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        sActivity = this;

        String scriptStackJson = getPreferences(Context.MODE_PRIVATE).getString("ScriptStack", "");
        String[] scripts = (new Gson()).fromJson(scriptStackJson, String[].class);
        if(scripts != null) {
            mScriptStack.clear();
            Collections.addAll(mScriptStack, scripts);
            mScriptStackIndex = mScriptStack.size();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        sActivity = null;

        String scriptStackJson = (new Gson()).toJson(mScriptStack);
        SharedPreferences.Editor sharedPreferencesEditor = getPreferences(Context.MODE_PRIVATE).edit();
        sharedPreferencesEditor.putString("ScriptStack", scriptStackJson);
        sharedPreferencesEditor.commit();
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_MENU) {
            run();
        }

        if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {
            popScriptStackUp(mScriptEditText);
        }

        if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
            popScriptStackDown(mScriptEditText);
        }

        return super.onKeyUp(keyCode, event);
    }

    @DoNotStrip
    public static void onGCEvent(final String extraInfo) {
        if(sActivity != null) {
            sActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    sActivity.mAdapter.addEvent("GC: " + extraInfo);
                }
            });
        }
    }

    static native HybridData initHybrid();
    native String nativeEvalScript(String s);
    native String nativeHeapStats();
    native void nativeCollect(String s);
}