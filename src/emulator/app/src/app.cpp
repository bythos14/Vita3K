// Vita3K emulator project
// Copyright (C) 2018 Vita3K team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <app/app_functions.h>

#include <bridge/imgui_impl_sdl_gl3.h>
#include <config/version.h>
#include <host/functions.h>
#include <host/sfo.h>
#include <host/state.h>
#include <kernel/functions.h>
#include <kernel/state.h>
#include <kernel/thread/thread_functions.h>
#include <touch/touch.h>
#include <util/find.h>
#include <util/log.h>

#ifdef USE_DISCORD_RICH_PRESENCE
#include <app/discord.h>
#endif

#include <SDL.h>
#include <glutil/gl.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

namespace app {

static void handle_window_event(HostState &state, const SDL_WindowEvent event) {
    switch (static_cast<SDL_WindowEventID>(event.event)) {
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        update_viewport(state);
        break;
    default:
        break;
    }
}

bool handle_events(HostState &host) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSdlGL3_ProcessEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            stop_all_threads(host.kernel);
            host.gxm.display_queue.abort();
            host.display.abort.exchange(true);
            host.display.condvar.notify_all();
            return false;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_g) {
                auto &display = host.display;

                // toggle gui state
                bool old_imgui_render = display.imgui_render.load();
                while (!display.imgui_render.compare_exchange_weak(old_imgui_render, !old_imgui_render)) {
                }
            }
            if (event.key.keysym.sym == SDLK_t) {
                toggle_touchscreen();
            }

        case SDL_WINDOWEVENT:
            handle_window_event(host, event.window);
            break;

        case SDL_FINGERDOWN:
            handle_touch_event(event.tfinger);
            break;

        case SDL_FINGERMOTION:
            handle_touch_event(event.tfinger);
            break;

        case SDL_FINGERUP:
            handle_touch_event(event.tfinger);
            break;
        }
    }

    return true;
}

void error_dialog(const std::string &message, SDL_Window *window) {
    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), window) < 0) {
        LOG_ERROR("SDL Error: {}", message);
    }
}

ExitCode run_app(HostState &host, Ptr<const void> &entry_point) {
    const CallImport call_import = [&host](CPUState &cpu, uint32_t nid, SceUID main_thread_id) {
        ::call_import(host, cpu, nid, main_thread_id);
    };

    const SceUID main_thread_id = create_thread(entry_point, host.kernel, host.mem, host.io.title_id.c_str(), SCE_KERNEL_DEFAULT_PRIORITY_USER, static_cast<int>(SCE_KERNEL_STACK_SIZE_USER_MAIN),
        call_import, false);

    if (main_thread_id < 0) {
        error_dialog("Failed to init main thread.", host.window.get());
        return InitThreadFailed;
    }

    const ThreadStatePtr main_thread = find(main_thread_id, host.kernel.threads);

    // Run `module_start` export (entry point) of loaded libraries
    for (auto &mod : host.kernel.loaded_modules) {
        const auto module = mod.second;
        const auto module_start = module->module_start;
        const auto module_name = module->module_name;

        if (std::string(module->path) == EBOOT_PATH_ABS)
            continue;

        LOG_DEBUG("Running module_start of library: {}", module_name);

        Ptr<void> argp = Ptr<void>();
        const SceUID module_thread_id = create_thread(module_start, host.kernel, host.mem, module_name, SCE_KERNEL_DEFAULT_PRIORITY_USER, static_cast<int>(SCE_KERNEL_STACK_SIZE_USER_DEFAULT),
            call_import, false);
        const ThreadStatePtr module_thread = find(module_thread_id, host.kernel.threads);
        const auto ret = run_on_current(*module_thread, module_start, 0, argp);
        module_thread->to_do = ThreadToDo::exit;
        module_thread->something_to_do.notify_all(); // TODO Should this be notify_one()?
        host.kernel.running_threads.erase(module_thread_id);
        host.kernel.threads.erase(module_thread_id);

        LOG_INFO("Module {} (at \"{}\") module_start returned {}", module_name, module->path, log_hex(ret));
    }

    if (start_thread(host.kernel, main_thread_id, 0, Ptr<void>()) < 0) {
        error_dialog("Failed to run main thread.", host.window.get());
        return RunThreadFailed;
    }

    return Success;
}

void set_window_title(HostState &host) {
    const uint32_t sdl_ticks_now = SDL_GetTicks();
    const uint32_t ms = sdl_ticks_now - host.sdl_ticks;
    if (ms >= 1000 && host.frame_count > 0) {
        const std::uint32_t fps = static_cast<std::uint32_t>((host.frame_count * 1000) / ms);
        const std::uint32_t ms_per_frame = ms / static_cast<std::uint32_t>(host.frame_count);
        std::ostringstream title;
        title << window_title << " | " << host.game_title << " (" << host.io.title_id << ") | " << ms_per_frame << " ms/frame (" << fps << " frames/sec)";
        SDL_SetWindowTitle(host.window.get(), title.str().c_str());
        host.sdl_ticks = sdl_ticks_now;
        host.frame_count = 0;
    }
}

} // namespace app
