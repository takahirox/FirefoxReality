package org.mozilla.vrbrowser;

import android.app.Activity;
import android.content.ComponentName;
import android.os.Bundle;
import android.util.Log;

import com.google.vr.ndk.base.DaydreamApi;

public class LaunchActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.e("VRB", "LaunchActivity onCreate");
        DaydreamApi daydream = DaydreamApi.create(this);
        daydream.launchInVr(new ComponentName("org.mozilla.vrbrowser", "org.mozilla.vrbrowser.VRBrowserActivity"));
    }
}
