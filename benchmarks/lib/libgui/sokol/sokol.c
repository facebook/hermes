/*
* Copyright (c) Tzvetan Mikov.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
 */

#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_time.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

// Must be separate to avoid reordering.
#include "sokol_debugtext.h"
