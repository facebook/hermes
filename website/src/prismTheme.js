/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const theme = {
  plain: {
    color: '#FFFFFF',
    background: '#242526',
  },
  styles: [
    {
      types: ['property'],
      style: {
        color: '#2aa198',
      },
    },
    {
      types: ['attr-name', 'comment', 'prolog', 'doctype', 'cdata'],
      style: {
        color: '#93a1a1',
      },
    },

    {
      types: ['punctuation'],
      style: {
        color: '#657b83',
      },
    },
    {
      types: ['namespace'],
      style: {
        opacity: 0.7,
      },
    },
    {
      types: ['selector', 'char', 'builtin', 'url'],
      style: {
        color: '#2aa198',
      },
    },
    {
      types: ['entity'],
      style: {
        color: '#2aa198',
      },
    },
    {
      types: ['atrule', 'inserted'],
      style: {
        color: '#859900',
      },
    },
    {
      types: ['important', 'variable', 'deleted'],
      style: {
        color: '#cb4b16',
      },
    },
    {
      types: ['important', 'bold'],
      style: {
        fontWeight: 'bold',
      },
    },
    {
      types: ['italic'],
      style: {
        fontStyle: 'italic',
      },
    },
    {
      types: ['entity'],
      style: {
        cursor: 'help',
      },
    },
    {
      types: ['attr-name', 'keyword'],
      style: {
        color: '#c5a5c5',
      },
    },
    {
      types: ['string', 'regex', 'attr-value'],
      style: {
        color: '#8dc891',
      },
    },
    {
      types: ['number', 'constant', 'symbol'],
      style: {
        color: '#5a9bcf',
      },
    },
    {
      types: ['boolean'],
      style: {
        color: '#ff8b50',
      },
    },
    {
      types: ['class-name'],
      style: {
        color: '#fac863',
      },
    },
    {
      types: ['function'],
      style: {
        color: '#79b6f2',
      },
    },
    {
      types: ['operator', 'tag'],
      style: {
        color: '#fc929e',
      },
    },
  ],
};

module.exports = theme;
