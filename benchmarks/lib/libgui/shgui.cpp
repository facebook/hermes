/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gui.h"

#include "sokol_app.h"
#include "sokol_log.h"

#include <hermes/VM/static_h.h>
#include <hermes/hermes.h>

static SHRuntime *s_shRuntime = nullptr;
static facebook::hermes::HermesRuntime *s_hermes = nullptr;

static int s_width = 800, s_height = 600;

extern "C" void gui_setup(int width, int height) {
  s_width = width;
  s_height = height;
}

extern "C" SHUnit sh_export_demo;

void gui_on_main(sapp_desc *desc, int argc, char **argv) {
  s_shRuntime = _sh_init(argc, argv);
  s_hermes = _sh_get_hermes_runtime(s_shRuntime);
  if (!_sh_initialize_units(s_shRuntime, 1, &sh_export_demo))
    abort();

  desc->width = s_width;
  desc->height = s_height;
  desc->window_title = "Static Hermes UI";
}

void gui_on_init() {
  try {
    s_hermes->global()
        .getPropertyAsFunction(*s_hermes, "on_init")
        .call(*s_hermes);
  } catch (facebook::jsi::JSIException &e) {
    slog_func("ERROR", 1, 0, e.what(), __LINE__, __FILE__, nullptr);
    abort();
  }
}

void gui_on_cleanup() {
  if (s_shRuntime) {
    _sh_done(s_shRuntime);
    s_shRuntime = nullptr;
  }
}

void gui_on_event(const sapp_event *ev) {
  try {
    s_hermes->global()
        .getPropertyAsFunction(*s_hermes, "on_event")
        .call(
            *s_hermes,
            (double)ev->type,
            (double)ev->key_code,
            (double)ev->modifiers);
  } catch (facebook::jsi::JSIException &e) {
    slog_func("ERROR", 1, 0, e.what(), __LINE__, __FILE__, nullptr);
  }
}
void gui_on_frame(int width, int height, double secs) {
  try {
    s_hermes->global()
        .getPropertyAsFunction(*s_hermes, "on_frame")
        .call(*s_hermes, width, height, secs);
  } catch (facebook::jsi::JSIException &e) {
    slog_func("ERROR", 1, 0, e.what(), __LINE__, __FILE__, nullptr);
  }
}
