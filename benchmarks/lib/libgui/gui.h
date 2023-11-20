/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

struct sapp_desc;
struct sapp_event;
struct simgui_image_t;

#ifdef __cplusplus
extern "C" {
#endif

// Hooks that must be provided by the application.
void gui_on_main(sapp_desc *desc, int argc, char **argv);
void gui_on_init();
void gui_on_cleanup();
void gui_on_event(const sapp_event *ev);
void gui_on_frame(int width, int height, double secs);

// Functions that can be called by the application.
void register_blob(const char *name, const unsigned char *data, unsigned size);
int make_image(int w, int h, const unsigned char *data);
int load_image(const char *path);
int image_width(int index);
int image_height(int index);
const simgui_image_t *image_simgui_image(int index);
void free_image(int index);

#ifdef __cplusplus
}
#endif
