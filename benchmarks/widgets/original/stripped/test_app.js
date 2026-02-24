/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import {
  Widget as Widget,
  ComposedWidget as ComposedWidget,
} from "./widget.js";
import {
  Button as Button,
  Container as Container,
  Gltf as Gltf,
  Floater as Floater,
} from "./widgets.js";
import Context from "./context.js";
import {
  SIZES_SMALL as SIZES_SMALL,
  SIZES_LARGE as SIZES_LARGE,
  MODELS_SMALL as MODELS_SMALL,
  MODELS_LARGE as MODELS_LARGE,
} from "./test_app_data.js";
class ButtonAndModel extends ComposedWidget {
  constructor(data) {
    super();
    this.data = data;
  }
  render() {
    const children = [
      new Button(this.data.buttonSize),
      new Gltf(this.data.modelPath),
      new Floater(Math.sqrt(this.data.buttonSize)),
    ];
    return new Container(children);
  }
}

export class TestApp extends ComposedWidget {
  constructor(renderLarge) {
    super();
    this.renderLarge = renderLarge;
  }
  getWidgets(sizes, models) {
    if (sizes.length != models.length) {
      throw new Error("sizes and models must have same length");
    }
    return models.map((modelPath, index) => {
      const buttonSize = sizes[index];
      const widget = new ButtonAndModel({
        modelPath: modelPath,
        buttonSize: buttonSize,
      });
      widget.key = `${modelPath}_${buttonSize}`;
      return widget;
    });
  }
  getChildren() {
    if (this.renderLarge) {
      return this.getWidgets(SIZES_LARGE, MODELS_LARGE);
    }
    return this.getWidgets(SIZES_SMALL, MODELS_SMALL);
  }
  render() {
    const children = this.getChildren();
    return new Container(children);
  }
}
