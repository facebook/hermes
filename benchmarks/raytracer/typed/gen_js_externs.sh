#!/bin/bash

set -e

$ffigen js js_externs.c sokol_imgui.h,sokol_time.h,cimgui.h > js_externs.js
$ffigen cwrap js_externs.c sokol_imgui.h,sokol_time.h,cimgui.h > tmp-cwrap.c

cat << "EOF" - tmp-cwrap.c > js_externs_cwrap.c
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui/cimgui.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_imgui.h"
EOF

rm tmp-cwrap.c
