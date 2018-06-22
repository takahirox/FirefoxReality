package org.mozilla.vrbrowser;

import android.support.annotation.Nullable;

public interface WidgetManagerDelegate {
    interface Listener {
        void onWidgetResize(Widget aWidget);
    }
    int newWidgetHandle();
    void addWidget(Widget aWidget);
    void updateWidget(Widget aWidget);
    void removeWidget(Widget aWidget);
    void startWidgetResize(Widget aWidget);
    void resetWidgetResize(Widget aWidget, float aWorldWidth, float aWorldHeight);
    void finishWidgetResize(Widget aWidget, boolean aCommitChanges);
    void addListener(WidgetManagerDelegate.Listener aListener);
    void removeListener(WidgetManagerDelegate.Listener aListener);

}
