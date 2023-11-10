/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 */

const s_vecs = calloc(_sizeof_ImVec2 * 4);

function drawImage(x: number, y: number, w: number, h: number, image: Image): void {
  // min
  set_ImVec2_x(s_vecs, x);
  set_ImVec2_y(s_vecs, y);
  // max
  set_ImVec2_x(_sh_ptr_add(s_vecs, _sizeof_ImVec2), x + w);
  set_ImVec2_y(_sh_ptr_add(s_vecs, _sizeof_ImVec2), y + h);
  // uv_min
  set_ImVec2_x(_sh_ptr_add(s_vecs, 2 * _sizeof_ImVec2), 0);
  set_ImVec2_y(_sh_ptr_add(s_vecs, 2 * _sizeof_ImVec2), 0);
  // uv_max
  set_ImVec2_x(_sh_ptr_add(s_vecs, 3 * _sizeof_ImVec2), 1);
  set_ImVec2_y(_sh_ptr_add(s_vecs, 3 * _sizeof_ImVec2), 1);

  _ImDrawList_AddImage(
      _igGetBackgroundDrawList_Nil(),
      _simgui_imtextureid(image.simguiImage),
      s_vecs,
      _sh_ptr_add(s_vecs, _sizeof_ImVec2),
      _sh_ptr_add(s_vecs, _sizeof_ImVec2 * 2),
      _sh_ptr_add(s_vecs, _sizeof_ImVec2 * 3),
      IM_COL32(255, 255, 255, 255)
  );
}

function fillRect(x: number, y: number, w: number, h: number, r: number, g: number, b: number, a: number): void {
  // min
  set_ImVec2_x(s_vecs, x);
  set_ImVec2_y(s_vecs, y);
  // max
  set_ImVec2_x(_sh_ptr_add(s_vecs, _sizeof_ImVec2), x + w);
  set_ImVec2_y(_sh_ptr_add(s_vecs, _sizeof_ImVec2), y + h);

  _ImDrawList_AddRectFilled(
      _igGetBackgroundDrawList_Nil(),
      s_vecs,
      _sh_ptr_add(s_vecs, _sizeof_ImVec2),
      IM_COL32(255 * r, 255 * g, 255 * b, 255 * a),
      0.0,
      0
  );
}

let image : Image;

globalThis.on_init = function on_init(): void {
  let w = 256;
  let h = 256;
  let buf = allocTmp(w * h * 4);
  let t1 = performance.now();
  exec(w, h, buf);
  print("exec time: ", (performance.now() - t1), "ms");
  image = new Image(w, h, buf);
}

globalThis.on_frame = function on_frame(width: number, height: number, curTime: number): void {
  flushAllocTmp();
  drawImage(0, 0, width, height, image);
}

globalThis.on_event = function on_event(type: number, key_code: number, modifiers: number): void {
  flushAllocTmp();
}

_gui_setup(1024, 768);
