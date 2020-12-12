/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const path = require('path');

module.exports = {
  title: 'Hermes',
  tagline: 'JavaScript engine optimized for React Native',
  organizationName: 'facebook',
  projectName: 'hermes',
  url: 'https://hermesengine.dev',
  baseUrl: '/',
  favicon: 'img/favicon.ico',
  themeConfig: {
    navbar: {
      title: 'Hermes',
      logo: {
        alt: 'Hermes Logo',
        src: 'img/logo.svg',
      },
      items: [
        { to: 'playground/', label: 'Playground', position: 'right' },
        { to: 'docs/building-and-running', label: 'Docs', position: 'right' },
        {
          href: 'https://github.com/facebook/hermes',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            {
              label: 'Building and Running Hermes',
              to: 'docs/building-and-running',
            },
            {
              label: 'Building Hermes with Emscripten',
              to: 'docs/emscripten',
            },
          ],
        },
        {
          title: 'Integrations',
          items: [
            {
              label: 'Using Hermes with React Native',
              to: 'https://reactnative.dev/docs/hermes',
            },
            {
              label: 'Using Hermes custom build with React Native',
              to: 'docs/react-native-integration',
            },
          ],
        },
        {
          title: 'Social',
          items: [
            {
              label: 'GitHub',
              to: 'https://github.com/facebook/hermes',
            },
            {
              label: 'Twitter',
              to: 'https://twitter.com/HermesEngine',
            },
          ],
        },
      ],
      logo: {
        alt: 'Facebook Open Source Logo',
        src: 'img/oss_logo.png',
        href: 'https://opensource.facebook.com',
      },
      copyright: `Copyright © ${new Date().getFullYear()} Facebook, Inc. Built with Docusaurus.`,
    },
    prism: {
      defaultLanguage: 'shell',
      theme: require('./src/prismTheme'),
    }
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          showLastUpdateAuthor: false,
          showLastUpdateTime: true,
          editUrl:
            'https://github.com/facebook/hermes/blob/master/website/',
          path: '../doc',
          sidebarPath: require.resolve('./sidebars.json'),
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
  clientModules: [
    require.resolve('./node_modules/normalize.css/normalize.css'),
  ],
  plugins: [
    path.join(__dirname, '/plugins/monaco-editor'),
    path.join(__dirname, '/plugins/case-sensitive-paths')
  ],
};
