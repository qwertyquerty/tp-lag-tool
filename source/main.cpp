/**
 * @file main.cpp
 * @author Madeline (qwertytrogi@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-05-01
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <display/console.h>     // Contains a very neat helper class to print to the console
#include <main.h>
#include <patch.h>     // Contains code for hooking into a function
#include <tp/f_ap_game.h>
#include <gc_wii/os.h>
#include <gc_wii/OSTime.h>
#include <cstring>
#include <cstdio>
#include <tp/m_do_controller_pad.h>

namespace mod
{
    Mod* gMod = nullptr;

    void main()
    {
        Mod* mod = new Mod();
        mod->init();
    }

    void exit() {}

    Mod::Mod(): c( 0 )
    {
        total_frames = 0;
        total_lag = 0;
        last_os_time_us = 0;
        expected_frame_time_us = 33367;
    }

    void Mod::init()
    {
        gMod = this;
        // Hook the function that runs each frame
        libtp::display::setConsoleColor(0, 0, 0, 0);

        return_fapGm_Execute = libtp::patch::hookFunction( libtp::tp::f_ap_game::fapGm_Execute, []() { return gMod->procNewFrame(); } );
    }

    void Mod::procNewFrame()
    {
        // This runs BEFORE the original (hooked) function (fapGm_Execute)

        // we can do whatever stuff we like... counting for example:

        uint64_t cur_os_time_us = getOSTimeUs();
        
        libtp::display::print(0, "Madeline's Lag Tool");
        libtp::display::print(1, "");
        
        int32_t frame_time = cur_os_time_us - last_os_time_us;
        int32_t frame_lag = frame_time - expected_frame_time_us;
        frame_lag = frame_lag < 0 ? 0 : frame_lag;

        if (measuring) {
            total_lag += frame_lag;
            total_frames++;

            libtp::display::print(2, "R+Y to stop");

            if (
                (libtp::tp::m_do_controller_pad::cpadInfo[0].mButtonFlags & libtp::tp::m_do_controller_pad::Button_R) &&
                (libtp::tp::m_do_controller_pad::cpadInfo[0].mButtonFlags & libtp::tp::m_do_controller_pad::Button_Y)
            ) {
                measuring = false;
            }
        }
        else {
            libtp::display::print(2, "R+X to start");

            if (
                (libtp::tp::m_do_controller_pad::cpadInfo[0].mButtonFlags & libtp::tp::m_do_controller_pad::Button_R) &&
                (libtp::tp::m_do_controller_pad::cpadInfo[0].mButtonFlags & libtp::tp::m_do_controller_pad::Button_X)
            ) {
                total_lag = 0;
                total_frames = 0;
                measuring = true;
            }
        }

        char total_frames_line[32];
        char frame_time_line[32];
        char frame_lag_line[32];
        char total_lag_line[32];
        char avg_lag_line[32];

        sprintf(total_frames_line, "Frames:     %d", total_frames);
        sprintf(total_lag_line,    "Total Lag:  %.1fms", (float)total_lag/1000.0f);

        if (total_frames != 0) {
            sprintf(avg_lag_line,      "Avg Lag:    %.1fms", (float)total_lag/(float)total_frames/1000.0f);
        } else {
            sprintf(avg_lag_line,      "Avg Lag:    ...");
        }

        sprintf(frame_time_line,   "Frame Time: %.1fms", (float)frame_time/1000.0f);
        sprintf(frame_lag_line,    "Frame Lag:  %.1fms", (float)frame_lag/1000.0f);

        libtp::display::print(4, total_frames_line);
        libtp::display::print(5, frame_time_line);
        libtp::display::print(6, frame_lag_line);
        libtp::display::print(7, total_lag_line);
        libtp::display::print(8, avg_lag_line);

        last_os_time_us = cur_os_time_us;
 
        return return_fapGm_Execute();
    }

    uint64_t Mod::getOSTimeUs() {
        return libtp::gc_wii::os_time::OSGetTime() * 4'000'000 / libtp::gc_wii::os::__OSBusClock;
    }
}     // namespace mod
