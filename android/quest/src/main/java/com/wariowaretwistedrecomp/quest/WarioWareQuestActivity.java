package com.wariowaretwistedrecomp.quest;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import org.gbarecomp.GbaGameActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Quest 3 OpenXR entry activity. Ensures OpenXR loader is loaded prior to
 * SDL2 and libmain, stages bundled/external ROM data if needed, and starts the
 * SDL_main game thread inside an IMMERSIVE_HMD VR session.
 */
public class WarioWareQuestActivity extends GbaGameActivity {
    private static final String TAG = "WarioWareXR";
    private static final String ROM_NAME = "warioware_twisted_usa.gba";

    @Override
    protected String[] getLibraries() {
        return new String[] { "openxr_loader", "SDL2", "main" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        copyExternalRomIfPresent(this);
        super.onCreate(savedInstanceState);
    }

    static void copyExternalRomIfPresent(Context ctx) {
        File romDst = new File(ctx.getFilesDir(), "roms/" + ROM_NAME);
        if (romDst.isFile() && romDst.length() > 0) return;
        String[] candidates = new String[] {
            "/sdcard/WarioWareTwisted/" + ROM_NAME,
            "/sdcard/WarioWareTwisted/WarioWare - Twisted! (USA).gba",
            "/sdcard/Download/WarioWare - Twisted! (USA).gba",
        };
        for (String path : candidates) {
            File src = new File(path);
            if (!src.isFile() || src.length() == 0) continue;
            File parent = romDst.getParentFile();
            if (parent != null && !parent.isDirectory()) parent.mkdirs();
            try (FileInputStream in = new FileInputStream(src);
                 FileOutputStream out = new FileOutputStream(romDst)) {
                byte[] buf = new byte[1024 * 1024];
                int n;
                while ((n = in.read(buf)) >= 0) {
                    if (n > 0) out.write(buf, 0, n);
                }
                Log.i(TAG, "Copied ROM from " + path + " -> " + romDst.getAbsolutePath());
                return;
            } catch (IOException e) {
                Log.w(TAG, "Failed copying ROM from " + path + ": " + e.getMessage());
            }
        }
    }
}
