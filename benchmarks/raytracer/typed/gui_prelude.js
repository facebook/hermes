/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 */

const _gui_setup = $SHBuiltin.extern_c({}, function gui_setup(width: c_int, height: c_int): void {
});
const _get_bg_color = $SHBuiltin.extern_c({}, function get_bg_color(): c_ptr {
  throw 0;
});

const _make_image = $SHBuiltin.extern_c({}, function make_image(w: c_int, h: c_int, data: c_ptr): c_int {
  return 0;
});
const _image_width = $SHBuiltin.extern_c({}, function image_width(image: c_int): c_int {
  return 0;
});
const _image_height = $SHBuiltin.extern_c({}, function image_height(image: c_int): c_int {
  return 0;
});
const _image_simgui_image = $SHBuiltin.extern_c({}, function image_simgui_image(image: c_int): c_ptr {
  throw 0;
});

class Performance {
  now(): number {
    return _stm_ms(_stm_now());
  }
}

let performance = new Performance();

class Image {
  handle: number;
  width: number;
  height: number;
  simguiImage: c_ptr;

  constructor(w: number, h: number, data: c_ptr) {
    this.handle = _make_image(w, h, data);
    this.width = w;
    this.height = h;
    this.simguiImage = _image_simgui_image(this.handle);
  }
}

const USE_GUI = true;
