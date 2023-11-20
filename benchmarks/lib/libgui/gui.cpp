/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_time.h"

#include "sokol_imgui.h"

// Must be separate to avoid reordering.
#include "sokol_debugtext.h"

#include "gui.h"

#include <vector>

struct Blob {
  const char *name;
  const unsigned char *data;
  unsigned size;
};
static constexpr unsigned kMaxBlobs = 32;
static Blob s_blobs[kMaxBlobs];
static unsigned s_numBlobs = 0;

extern "C" void
register_blob(const char *name, const unsigned char *data, unsigned size) {
  if (s_numBlobs >= kMaxBlobs) {
    slog_func("ERROR", 1, 0, "Too many blobs", __LINE__, __FILE__, nullptr);
    abort();
  }
  s_blobs[s_numBlobs++] = {name, data, size};
}
static const unsigned char *find_blob(const char *name, unsigned *size) {
  for (unsigned i = 0; i < s_numBlobs; ++i) {
    if (strcmp(s_blobs[i].name, name) == 0) {
      *size = s_blobs[i].size;
      return s_blobs[i].data;
    }
  }
  return nullptr;
}

static sg_sampler s_sampler = {};

class Image {
 public:
  int w_ = 0, h_ = 0;
  sg_image image_ = {};
  simgui_image_t simguiImage_ = {};

  explicit Image(int w, int h, const unsigned char *data) : w_(w), h_(h) {
    image_ = sg_make_image(sg_image_desc{
        .width = w_,
        .height = h_,
        .data{.subimage[0][0] = {.ptr = data, .size = (size_t)w_ * h_ * 4}},
    });

    simguiImage_ = simgui_make_image(simgui_image_desc_t{image_, s_sampler});
  }

  ~Image() {
    simgui_destroy_image(simguiImage_);
    sg_destroy_image(image_);
  }
};

static std::vector<std::unique_ptr<Image>> s_images{};

static bool s_started = false;
static uint64_t s_start_time = 0;
static uint64_t s_last_fps_time = 0;
static double s_fps = 0;

extern "C" int make_image(int w, int h, const unsigned char *data) {
  s_images.emplace_back(std::make_unique<Image>(w, h, data));
  return s_images.size() - 1;
}
extern "C" int image_width(int index) {
  if (index < 0 || index >= s_images.size()) {
    slog_func(
        "ERROR", 1, 0, "Invalid image index", __LINE__, __FILE__, nullptr);
    return 0;
  }
  return s_images[index]->w_;
}
extern "C" int image_height(int index) {
  if (index < 0 || index >= s_images.size()) {
    slog_func(
        "ERROR", 1, 0, "Invalid image index", __LINE__, __FILE__, nullptr);
    return 0;
  }
  return s_images[index]->h_;
}
extern "C" const simgui_image_t *image_simgui_image(int index) {
  if (index < 0 || index >= s_images.size()) {
    slog_func(
        "ERROR", 1, 0, "Invalid image index", __LINE__, __FILE__, nullptr);
    return 0;
  }
  return &s_images[index]->simguiImage_;
}

static void app_init() {
  stm_setup();

  sg_desc desc = {.context = sapp_sgcontext(), .logger.func = slog_func};
  sg_setup(&desc);
  simgui_setup(simgui_desc_t{});

  s_sampler = sg_make_sampler(sg_sampler_desc{
      .min_filter = SG_FILTER_LINEAR,
      .mag_filter = SG_FILTER_LINEAR,
  });

  sdtx_desc_t sdtx_desc = {
      .fonts = {sdtx_font_kc854()}, .logger.func = slog_func};
  sdtx_setup(&sdtx_desc);

  gui_on_init();
}

static void app_cleanup() {
  s_images.clear();
  simgui_shutdown();
  sdtx_shutdown();
  sg_shutdown();
  gui_on_cleanup();
}

static void app_event(const sapp_event *ev) {
  if (ev->type == SAPP_EVENTTYPE_KEY_DOWN && ev->key_code == SAPP_KEYCODE_Q &&
      (ev->modifiers & SAPP_MODIFIER_SUPER)) {
    sapp_request_quit();
    return;
  }

  gui_on_event(ev);

  if (simgui_handle_event(ev))
    return;
}

static float s_bg_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
extern "C" float *get_bg_color() {
  return s_bg_color;
}

static void app_frame() {
  uint64_t now = stm_now();

  if (!s_started) {
    s_started = true;
    s_start_time = now;
    s_last_fps_time = now;
  } else {
    // Update FPS every second
    uint64_t diff = stm_diff(now, s_last_fps_time);
    if (diff > 1000000000) {
      s_fps = 1.0 / sapp_frame_duration(); // stm_sec(diff);
      s_last_fps_time = now;
    }
  }

  simgui_new_frame({
      .width = sapp_width(),
      .height = sapp_height(),
      .delta_time = sapp_frame_duration(),
      .dpi_scale = sapp_dpi_scale(),
  });

  // Setup pass action to clear the framebuffer with yellow color
  sg_pass_action pass_action = {
      .colors[0] = {
          .load_action = SG_LOADACTION_CLEAR,
          .clear_value = {
              s_bg_color[0], s_bg_color[1], s_bg_color[2], s_bg_color[3]}}};

  // Begin and end pass
  sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());

  gui_on_frame(
      sapp_width(), sapp_height(), stm_sec(stm_diff(now, s_start_time)));

  simgui_render();
  sdtx_canvas((float)sapp_width(), (float)sapp_height());
  sdtx_printf("FPS: %d", (int)(s_fps + 0.5));
  sdtx_draw();
  sg_end_pass();
  sg_commit();
}

sapp_desc sokol_main(int argc, char *argv[]) {
  sapp_desc desc = {};
  desc.init_cb = app_init;
  desc.frame_cb = app_frame;
  desc.cleanup_cb = app_cleanup;
  desc.event_cb = app_event;
  desc.width = 800;
  desc.height = 600;
  desc.window_title = "libgui";
  desc.logger.func = slog_func;

  gui_on_main(&desc, argc, argv);
  return desc;
}
