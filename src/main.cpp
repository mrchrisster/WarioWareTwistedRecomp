#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "mobile_platform.h"
#include "runtime.h"

extern "C" int g_layered_unify_objects;

#if defined(GBAGAME_RECOMP_UI)
#include "game_launcher_boot.h"
#endif

#ifndef GBARECOMP_BUILTIN_NAME
#define GBARECOMP_BUILTIN_NAME "GBA cartridge"
#endif
#ifndef GBARECOMP_BUILTIN_SHA1
#define GBARECOMP_BUILTIN_SHA1 ""
#endif
#ifndef GBARECOMP_BUILTIN_CRC32
#define GBARECOMP_BUILTIN_CRC32 0
#endif
#ifndef GBARECOMP_BUILTIN_REGION
#define GBARECOMP_BUILTIN_REGION ""
#endif
#ifndef GBARECOMP_DEFAULT_GAME_CONFIG
#define GBARECOMP_DEFAULT_GAME_CONFIG "game.toml"
#endif

namespace {

void set_environment_default(const char* name, const char* value) {
    if (std::getenv(name) != nullptr) return;
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 0);
#endif
}

void print_usage() {
    std::printf(
        "WarioWareTwistedRecomp [--bios <path>] [--rom <path>] "
        "[game.toml]\n"
        "The BIOS and ROM must match the SHA-1 identities in game.toml.\n"
        "Hold the left mouse button and drag horizontally to simulate gyro "
        "motion.\n"
        "Windowed play opens the recomp-ui launcher first; --no-launcher "
        "skips it once.\n"
        "Static recompilation is the default; set GBARECOMP_FORCE_INTERP=1 "
        "for the reference interpreter diagnostic backend.\n");
}

}  // namespace

int warioware_main(int argc, char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    // Android: private-storage layout, log file, staged game TOML and no
    // pre-boot launcher (shared gbarecomp mobile setup). No-op on desktop.
    gbarecomp::MobileProcessOptions mobile;
    mobile.game_config = GBARECOMP_DEFAULT_GAME_CONFIG;
    mobile.program_name = "./WarioWareTwistedRecomp";
    if (gbarecomp::mobile_prepare_process(args, mobile)) {
        args.emplace_back("--gyro-sensitivity");
        args.emplace_back("1.0");
    }
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 ||
            std::strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    // WarioWare: Twisted! legitimately spends long periods in polling loops.
    // The engine's instruction-level hang watchdog treats that as suspicious
    // and its trace capture can starve interactive audio. Keep it available as
    // an explicit diagnostic override, but disable it for normal play.
    set_environment_default("GBARECOMP_HANG_WATCHDOG", "0");

    // The engine's custom clock-domain bridge currently garbles this title's
    // otherwise-correct 65536 Hz mixer stream. Queue it directly and let SDL
    // perform only the host-device conversion.
    set_environment_default("GBARECOMP_AUDIO_DIRECT", "1");

    gbarecomp::RunOptions opts;
    opts.builtin_game_name = GBARECOMP_BUILTIN_NAME;
    opts.builtin_rom_sha1 = GBARECOMP_BUILTIN_SHA1;
    opts.builtin_rom_crc32 = GBARECOMP_BUILTIN_CRC32;
    opts.launcher_region = GBARECOMP_BUILTIN_REGION;
    opts.launcher_game_config = GBARECOMP_DEFAULT_GAME_CONFIG;
    opts.launcher_save_path =
        "saves/warioware_twisted_usa.sav";
    opts.launcher_expose_gyro = true;
    // The validated controller/phone response felt best at the former 0.75x
    // setting. Treat that response as WarioWare's authored 1.00x baseline on
    // every platform so the player-facing slider is a relative multiplier.
    opts.gyro_sensitivity_calibration = 0.75f;
#if defined(__ANDROID__)
    opts.ui_touch_friendly = true;
#endif
#if defined(GBARECOMP_QUEST_XR)
    // Quest VR: shared Wario Land 4 VR runtime features.
    // Sprites stay on their own GBA priority planes (Mother 3's object clamp hid them).
    g_layered_unify_objects = 0;
    // OpenSL ES instead of SDL's Java AudioTrack driver, whose thread stalls break up the sound.
    setenv("SDL_AUDIODRIVER", "openslES", 0);
    // Resume where the player left off (suspend state / newest state); AUTO RESUME in the menu.
    opts.resume_suspend_state_on_launch = true;
    opts.resume_newest_state_on_launch = true;
    opts.snapshot_on_battery_flush = true;
    // Performance cores, sustained-high CPU clocks, no background diagnostic replay recorder.
    setenv("GBARECOMP_BIG_CORES", "1", 0);
    setenv("GBARECOMP_XR_CPU_HIGH", "1", 0);
    setenv("GBARECOMP_DIAGNOSTICS", "0", 0);
    // Keep the screen distance / size the player last set with the grips.
    setenv("GBARECOMP_XR_REMEMBER_VIEW", "1", 0);
    // VR options menu on the Menu button: save/load state, 3D depth, background, edges,
    // auto resume, recenter, reset screen size.
    setenv("GBARECOMP_XR_MENU", "1", 0);
    setenv("GBARECOMP_XR_TITLE", "WARIOWARE TWISTED VR", 0);
    // Shadow box like Wario Land 4: 24 GBA px of edge-extended margin on every layer and a dark
    // frame right in front of the front layer (EDGES = SHADOW BOX), or every layer clipped to
    // the front layer's rectangle (EDGES = WINDOW). Without it the deeper layers' edges float
    // free of the front layer.
    setenv("GBARECOMP_XR_SHADOWBOX", "24", 0);
    setenv("GBARECOMP_XR_BOXWALLS", "0", 0);
    setenv("GBARECOMP_XR_WINDOW", "0", 0);
    setenv("GBARECOMP_XR_DARK", "1", 0);
    // Cartridge gyro from wrist twist on the Touch controller (L-stick click swaps hand).
    setenv("GBARECOMP_XR_GYRO", "1", 0);
#endif

#if defined(GBAGAME_RECOMP_UI)
    if (game_launcher_preboot(args, opts)) return 0;
#endif
    std::vector<char*> av;
    av.reserve(args.size());
    for (auto& arg : args) av.push_back(arg.data());
    return gbarecomp::run_game(static_cast<int>(av.size()), av.data(), opts);
}

#if defined(__ANDROID__)
extern "C" int SDL_main(int argc, char** argv) {
    // Desktop-sized host stack for recompiled code (see mobile_platform.h).
    return gbarecomp::mobile_run_with_stack(warioware_main, argc, argv);
}
#else
int main(int argc, char** argv) {
    return warioware_main(argc, argv);
}
#endif
