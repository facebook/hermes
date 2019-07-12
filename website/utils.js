/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const path = require('path');

const fs = require('fs-extra');
const recursiveReaddir = require('recursive-readdir');

const baseUrlRegExp = /"\/([a-z0-9._/-]*?\.(css|html|js|ico|svg|png|jpeg))"/ig;
const baseUrlPattern = baseUrlRegExp.source;

const buildDirectory = path.join(__dirname, 'build');

const relativify = (content, filePath) =>
  content.replace(baseUrlRegExp, (match, filepath) => {
    return '"' + filepath + '"';
  });

const websiteTextualFileExtensions = ['.css', '.js', '.html', '.xml'];

const isNotWebsiteTextualFile = (filePath, stats) =>
  !(stats.isDirectory() || websiteTextualFileExtensions.includes(path.extname(filePath)));

const postProcess = async () => {
  const filePaths = await recursiveReaddir(buildDirectory, [isNotWebsiteTextualFile]);
  await Promise.all(
    filePaths.map(async filePath => {
      const content = await fs.readFile(filePath);
      const relativePath = path.relative(buildDirectory, filePath);
      await fs.writeFile(filePath, relativify(String(content), relativePath));
    })
  );
};

module.exports = {
  baseUrlPattern,
  postProcess,
};

