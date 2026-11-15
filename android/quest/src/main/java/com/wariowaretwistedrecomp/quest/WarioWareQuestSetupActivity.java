package com.wariowaretwistedrecomp.quest;

import android.os.Bundle;

import org.gbarecomp.GbaSetupActivity;

/**
 * First-launch Quest setup activity. On clean BYO-ROM public builds this
 * opens in a Quest 2D panel so the user can select their legally obtained
 * WarioWare: Twisted! (USA) ROM via Android's document picker (or pick it up
 * automatically from /sdcard/Download or /sdcard/WarioWareTwisted), then
 * launches WarioWareQuestActivity into immersive OpenXR VR.
 */
public class WarioWareQuestSetupActivity extends GbaSetupActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        WarioWareQuestActivity.copyExternalRomIfPresent(this);
        super.onCreate(savedInstanceState);
    }

    @Override
    protected Class<?> gameActivityClass() {
        return WarioWareQuestActivity.class;
    }
}
