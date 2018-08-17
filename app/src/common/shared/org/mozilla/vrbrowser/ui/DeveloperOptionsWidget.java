/* -*- Mode: Java; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.vrbrowser.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.*;
import org.mozilla.vrbrowser.R;
import org.mozilla.vrbrowser.SettingsStore;
import org.mozilla.vrbrowser.WidgetPlacement;
import org.mozilla.vrbrowser.audio.AudioEngine;

public class DeveloperOptionsWidget extends UIWidget {

    private static final String LOGTAG = "VRB";

    private static final int RESTART_DIALOG_ID = 0;

    // Developer options default values
    private final static boolean REMOTE_DEBUGGING_DEFAULT = false;
    private final static boolean CONSOLE_LOGS_DEFAULT = false;
    private final static boolean ENV_OVERRIDE_DEFAULT = false;
    private final static boolean DESKTOP_VERSION_DEFAULT = false;
    private final static InputMode TOUCH_DEFAULT = InputMode.TOUCH;
    private final static int DENSITY_DEFAULT = 2;
    private final static int WINDOW_WIDTH_DEFAULT = 1024;
    private final static int WINDOW_HEIGHT_DEFAULT = 768;

    enum InputMode {
        MOUSE,
        TOUCH
    }

    private AudioEngine mAudio;
    private Switch mRemoteDebuggingSwitch;
    private Switch mConsoleLogsSwitch;
    private Switch mEnvOverrideSwitch;
    private Switch mDesktopVersionSwitch;
    private UIButton mBackButton;
    private RadioGroup mEventsRadio;
    private TextView mDensityButton;
    private TextView mWindowSizeButton;

    public DeveloperOptionsWidget(Context aContext) {
        super(aContext);
        initialize(aContext);
    }

    public DeveloperOptionsWidget(Context aContext, AttributeSet aAttrs) {
        super(aContext, aAttrs);
        initialize(aContext);
    }

    public DeveloperOptionsWidget(Context aContext, AttributeSet aAttrs, int aDefStyle) {
        super(aContext, aAttrs, aDefStyle);
        initialize(aContext);
    }

    private void initialize(Context aContext) {
        inflate(aContext, R.layout.developer_options, this);

        mAudio = AudioEngine.fromContext(aContext);

        mBackButton = findViewById(R.id.backButton);
        mBackButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }
                
                hide();
            }
        });

        mRemoteDebuggingSwitch = findViewById(R.id.developer_options_remote_debugging_switch);
        mRemoteDebuggingSwitch.setChecked(SettingsStore.getInstance(getContext()).isRemoteDebuggingEnabled(REMOTE_DEBUGGING_DEFAULT));
        mRemoteDebuggingSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                SettingsStore.getInstance(getContext()).setRemoteDebuggingEnabled(b);
            }
        });

        mConsoleLogsSwitch = findViewById(R.id.developer_options_show_console_switch);
        mConsoleLogsSwitch.setChecked(SettingsStore.getInstance(getContext()).isConsoleLogsEnabled(CONSOLE_LOGS_DEFAULT));
        mConsoleLogsSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                SettingsStore.getInstance(getContext()).setConsoleLogsEnabled(b);
            }
        });

        mEnvOverrideSwitch = findViewById(R.id.developer_options_env_override_switch);
        mEnvOverrideSwitch.setChecked(SettingsStore.getInstance(getContext()).isEnvironmentOverrideEnabled(ENV_OVERRIDE_DEFAULT));
        mEnvOverrideSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                SettingsStore.getInstance(getContext()).setEnvironmentOverrideEnabled(b);
            }
        });

        mDesktopVersionSwitch = findViewById(R.id.developer_options_desktop_version_switch);
        mDesktopVersionSwitch.setChecked(SettingsStore.getInstance(getContext()).isDesktopVersionEnabled(DESKTOP_VERSION_DEFAULT));
        mDesktopVersionSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                SettingsStore.getInstance(getContext()).setDesktopVersionEnabled(b);
            }
        });

        mEventsRadio = findViewById(R.id.radioEvents);
        RadioButton mTouchRadio = findViewById(R.id.radioTouch);
        RadioButton mMouseRadio = findViewById(R.id.radioMouse);
        InputMode inputMode = InputMode.values()[SettingsStore.getInstance(getContext()).getInputMode(TOUCH_DEFAULT.ordinal())];
        if (inputMode == InputMode.MOUSE) {
            mTouchRadio.setChecked(false);
            mMouseRadio.setChecked(true);

        } else if (inputMode == InputMode.TOUCH) {
            mTouchRadio.setChecked(true);
            mMouseRadio.setChecked(false);
        }
        mEventsRadio.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                if (checkedId == R.id.radioMouse) {
                    SettingsStore.getInstance(getContext()).setInputMode(InputMode.MOUSE.ordinal());

                } else if (checkedId == R.id.radioTouch) {
                    SettingsStore.getInstance(getContext()).setInputMode(InputMode.TOUCH.ordinal());
                }
            }
        });

        final TextView densityText = findViewById(R.id.densityText);
        densityText.setText(Integer.toString(SettingsStore.getInstance(getContext()).getDisplayDensity(DENSITY_DEFAULT)));
        final EditText densityEdit = findViewById(R.id.densityEdit);
        densityEdit.setText(Integer.toString(SettingsStore.getInstance(getContext()).getDisplayDensity(DENSITY_DEFAULT)));
        densityEdit.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int actionId, KeyEvent keyEvent) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    mDensityButton.callOnClick();
                    return true;
                }

                return false;
            }
        });
        mDensityButton = findViewById(R.id.densityEditButton);
        mDensityButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                if (densityEdit.getVisibility() == View.VISIBLE) {
                    densityText.setText(densityEdit.getText());
                    densityText.setVisibility(View.VISIBLE);
                    densityEdit.setVisibility(View.GONE);

                    createChild(RESTART_DIALOG_ID, RestartDialogWidget.class, false).show();

                } else {
                    densityText.setVisibility(View.GONE);
                    densityEdit.setVisibility(View.VISIBLE);
                }

                SettingsStore.getInstance(getContext()).setDisplayDensity(Integer.parseInt(densityText.getText().toString()));
            }
        });

        final TextView windowWidthText = findViewById(R.id.windowSizeWidthText);
        windowWidthText.setText(Integer.toString(SettingsStore.getInstance(getContext()).getWindowWidth(WINDOW_WIDTH_DEFAULT)));
        final EditText windowWidthEdit = findViewById(R.id.windowSizeWidthEdit);
        windowWidthEdit.setText(Integer.toString(SettingsStore.getInstance(getContext()).getWindowWidth(WINDOW_WIDTH_DEFAULT)));
        windowWidthEdit.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int actionId, KeyEvent keyEvent) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    mWindowSizeButton.callOnClick();
                    return true;
                }

                return false;
            }
        });
        final TextView windowHeightText = findViewById(R.id.windowSizeHeightText);
        windowHeightText.setText(Integer.toString(SettingsStore.getInstance(getContext()).getWindowHeight(WINDOW_HEIGHT_DEFAULT)));
        final EditText windowHeightEdit = findViewById(R.id.windowSizeHeightEdit);
        windowHeightEdit.setText(Integer.toString(SettingsStore.getInstance(getContext()).getWindowHeight(WINDOW_HEIGHT_DEFAULT)));
        windowHeightEdit.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int actionId, KeyEvent keyEvent) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    mWindowSizeButton.callOnClick();
                    return true;
                }

                return false;
            }
        });
        mWindowSizeButton = findViewById(R.id.windowSizeEditButton);
        mWindowSizeButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                if (mAudio != null) {
                    mAudio.playSound(AudioEngine.Sound.CLICK);
                }

                if (windowWidthEdit.getVisibility() == View.VISIBLE) {
                    windowWidthText.setText(windowWidthEdit.getText());
                    windowHeightText.setText(windowHeightEdit.getText());
                    windowWidthText.setVisibility(View.VISIBLE);
                    windowHeightText.setVisibility(View.VISIBLE);
                    windowWidthEdit.setVisibility(View.GONE);
                    windowHeightEdit.setVisibility(View.GONE);

                } else {
                    windowWidthText.setVisibility(View.GONE);
                    windowHeightText.setVisibility(View.GONE);
                    windowWidthEdit.setVisibility(View.VISIBLE);
                    windowHeightEdit.setVisibility(View.VISIBLE);
                }

                SettingsStore.getInstance(getContext()).setWindowWidth(Integer.parseInt(windowWidthText.getText().toString()));
                SettingsStore.getInstance(getContext()).setWindowHeight(Integer.parseInt(windowHeightText.getText().toString()));
            }
        });
    }

    @Override
    void initializeWidgetPlacement(WidgetPlacement aPlacement) {
        aPlacement.visible = false;
        aPlacement.width =  WidgetPlacement.dpDimension(getContext(), R.dimen.developer_options_width);
        aPlacement.height = WidgetPlacement.dpDimension(getContext(), R.dimen.developer_options_height);
        aPlacement.parentAnchorX = 0.5f;
        aPlacement.parentAnchorY = 0.5f;
        aPlacement.anchorX = 0.5f;
        aPlacement.anchorY = 0.5f;
        aPlacement.translationY = WidgetPlacement.unitFromMeters(getContext(), R.dimen.restart_dialog_world_y);
        aPlacement.translationZ = WidgetPlacement.unitFromMeters(getContext(), R.dimen.restart_dialog_world_z);
    }

    protected void onChildShown(int aChildId) {
        hide();
    }

    protected void onChildHidden(int aChildId) {
        show();
    }

}
