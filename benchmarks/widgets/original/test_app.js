/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {Widget, ComposedWidget} from './widget.js';
import {Button, Container, Gltf, Floater} from './widgets.js';
import Context from './context.js';
import {
  SIZES_SMALL,
  SIZES_LARGE,
  MODELS_SMALL,
  MODELS_LARGE,
} from './test_app_data.js';

type RenderData = {
  modelPath: string,
  buttonSize: number,
};

class ButtonAndModel extends ComposedWidget {
  data: RenderData;

  constructor(data: RenderData) {
    super();
    this.data = data;
  }

  render(): Widget {
    const children = [
      new Button(this.data.buttonSize),
      new Gltf(this.data.modelPath),
      new Floater(Math.sqrt(this.data.buttonSize)),
    ];

    return new Container(children);
  }
}

export class TestApp extends ComposedWidget {
  renderLarge: boolean;

  constructor(renderLarge: boolean) {
    super();
    this.renderLarge = renderLarge;
  }

  getWidgets(sizes: number[], models: string[]): Widget[] {
    if (sizes.length != models.length) {
      throw new Error('sizes and models must have same length');
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

  getChildren(): Widget[] {
    if (this.renderLarge) {
      return this.getWidgets(SIZES_LARGE, MODELS_LARGE);
    }
    return this.getWidgets(SIZES_SMALL, MODELS_SMALL);
  }

  render(): Widget {
    const children = this.getChildren();
    return new Container(children);
  }
}
