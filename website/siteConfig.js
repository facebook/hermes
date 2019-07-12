/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const {baseUrlPattern} = require('./utils');

const isBuilding = process.env.npm_lifecycle_script
  && process.env.npm_lifecycle_script === 'docusaurus-build'; 

const siteConfig = {
  // Keep `/` in watch mode.
  baseUrl: isBuilding ? baseUrlPattern : '/',
  // ...
};

module.exports = siteConfig;

