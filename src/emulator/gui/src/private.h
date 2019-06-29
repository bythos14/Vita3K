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

#pragma once

#include <gui/state.h>

#include <host/state.h>

#include <imgui.h>

struct HostState;

namespace gui {

#define RGBA_TO_FLOAT(r, g, b, a) ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f)

const ImVec4 GUI_COLOR_TEXT_MENUBAR = RGBA_TO_FLOAT(242, 150, 58, 255);
const ImVec4 GUI_COLOR_TEXT_MENUBAR_OPTIONS = RGBA_TO_FLOAT(242, 150, 58, 255);
const ImVec4 GUI_COLOR_TEXT_TITLE = RGBA_TO_FLOAT(247, 198, 51, 255);
const ImVec4 GUI_COLOR_TEXT = RGBA_TO_FLOAT(255, 255, 255, 255);
const ImVec4 GUI_COLOR_SEARCH_BAR_TEXT = RGBA_TO_FLOAT(200, 200, 200, 255);
const ImVec4 GUI_COLOR_SEARCH_BAR_BG = RGBA_TO_FLOAT(42, 42, 42, 255);

#undef RGBA_TO_FLOAT

void draw_main_menu_bar(GuiState &gui);
void draw_threads_dialog(HostState &host, GuiState &gui);
void draw_thread_details_dialog(HostState &host, GuiState &gui);
void draw_semaphores_dialog(HostState &host, GuiState &gui);
void draw_mutexes_dialog(HostState &host, GuiState &gui);
void draw_lw_mutexes_dialog(HostState &host, GuiState &gui);
void draw_lw_condvars_dialog(HostState &host, GuiState &gui);
void draw_condvars_dialog(HostState &host, GuiState &gui);
void draw_event_flags_dialog(HostState &host, GuiState &gui);
void draw_allocations_dialog(HostState &host, GuiState &gui);
void draw_disassembly_dialog(HostState &host, GuiState &gui);
void draw_shader_editor_dialog(HostState &host, GuiState &gui);
void draw_settings_dialog(HostState &host, GuiState &gui);
void draw_controls_dialog(HostState &host, GuiState &gui);
void draw_about_dialog(GuiState &gui);

void reevaluate_code(HostState &host, GuiState &gui);

} // namespace gui
