package org.mozilla.vrbrowser;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.StrictMode;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;

import org.mozilla.telemetry.TelemetryHolder;
import org.mozilla.vrbrowser.telemetry.TelemetryWrapper;


public class SettingsStore {

    private static final String LOGTAG = "VRB";

    private static SettingsStore mSettingsInstance;

    public static synchronized @NonNull
    SettingsStore getInstance(final @NonNull Context aContext) {
        if (mSettingsInstance == null) {
            mSettingsInstance = new SettingsStore(aContext);
        }

        return mSettingsInstance;
    }

    private Context mContext;
    private SharedPreferences mPrefs;
    // Enable telemetry by default (opt-out).
    private final static boolean enableCrashReportingByDefault = false;
    private final static boolean enableTelemetryByDefault = true;

    public SettingsStore(Context aContext) {
        mContext = aContext;
        mPrefs = PreferenceManager.getDefaultSharedPreferences(aContext);
    }

    public boolean isCrashReportingEnabled() {
        return mPrefs.getBoolean(mContext.getString(R.string.settings_key_crash), enableCrashReportingByDefault);
    }

    public void setCrashReportingEnabled(boolean isEnabled) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(mContext.getString(R.string.settings_key_crash), isEnabled);
        editor.commit();
    }

    public boolean isTelemetryEnabled() {
        // The first access to shared preferences will require a disk read.
        final StrictMode.ThreadPolicy threadPolicy = StrictMode.allowThreadDiskReads();
        try {
            return mPrefs.getBoolean(
                    mContext.getString(R.string.settings_key_telemetry), enableTelemetryByDefault);
        } finally {
            StrictMode.setThreadPolicy(threadPolicy);
        }
    }

    public void setTelemetryEnabled(boolean isEnabled) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(mContext.getString(R.string.settings_key_telemetry), isEnabled);
        editor.commit();

        // If the state of Telemetry is not the same, we reinitialize it.
        final boolean hasEnabled = isTelemetryEnabled();
        if (hasEnabled != isEnabled) {
            TelemetryWrapper.init(mContext);
        }

        TelemetryHolder.get().getConfiguration().setUploadEnabled(isEnabled);
        TelemetryHolder.get().getConfiguration().setCollectionEnabled(isEnabled);
    }

    public boolean isRemoteDebuggingEnabled(boolean aDefaultValue) {
        return mPrefs.getBoolean(
                mContext.getString(R.string.settings_key_console_logs), aDefaultValue);
    }

    public void setRemoteDebuggingEnabled(boolean isEnabled) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(mContext.getString(R.string.settings_key_remote_debugging), isEnabled);
        editor.commit();
    }

    public boolean isConsoleLogsEnabled(boolean aDefaultValue) {
        return mPrefs.getBoolean(
                mContext.getString(R.string.settings_key_console_logs), aDefaultValue);
    }

    public void setConsoleLogsEnabled(boolean isEnabled) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(mContext.getString(R.string.settings_key_console_logs), isEnabled);
        editor.commit();
    }

    public boolean isEnvironmentOverrideEnabled(boolean aDefaultValue) {
        return mPrefs.getBoolean(
                mContext.getString(R.string.settings_key_environment_override), aDefaultValue);
    }

    public void setEnvironmentOverrideEnabled(boolean isEnabled) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(mContext.getString(R.string.settings_key_environment_override), isEnabled);
        editor.commit();
    }

    public boolean isDesktopVersionEnabled(boolean aDefaultValue) {
        return mPrefs.getBoolean(
                mContext.getString(R.string.settings_key_desktop_version), aDefaultValue);
    }

    public void setDesktopVersionEnabled(boolean isEnabled) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(mContext.getString(R.string.settings_key_desktop_version), isEnabled);
        editor.commit();
    }

    public int getInputMode(int aDefaultValue) {
        return mPrefs.getInt(
                mContext.getString(R.string.settings_key_input_mode), aDefaultValue);
    }

    public void setInputMode(int aTouchMode) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putInt(mContext.getString(R.string.settings_key_input_mode), aTouchMode);
        editor.commit();
    }

    public int getDisplayDensity(int aDefaultValue) {
        return mPrefs.getInt(
                mContext.getString(R.string.settings_key_display_density), aDefaultValue);
    }

    public void setDisplayDensity(int aDensity) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putInt(mContext.getString(R.string.settings_key_display_density), aDensity);
        editor.commit();
    }

    public int getWindowWidth(int aDefaultValue) {
        return mPrefs.getInt(
                mContext.getString(R.string.settings_key_window_width), aDefaultValue);
    }

    public void setWindowWidth(int aWindowWidth) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putInt(mContext.getString(R.string.settings_key_window_width), aWindowWidth);
        editor.commit();
    }

    public int getWindowHeight(int aDefaultValue) {
        return mPrefs.getInt(
                mContext.getString(R.string.settings_key_window_height), aDefaultValue);
    }

    public void setWindowHeight(int aWindowHeight) {
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putInt(mContext.getString(R.string.settings_key_window_height), aWindowHeight);
        editor.commit();
    }

}
