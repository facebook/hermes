/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
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
        {to: 'docs/building-and-running', label: 'Docs', position: 'left'},
        {to: 'playground/', label: 'Playground', position: 'left'},
        // Please keep GitHub link to the right for consistency.
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
              label: 'Language Features',
              to: 'docs/language-features',
            },
            {
              label: 'Building and Running',
              to: 'docs/building-and-running',
            },
            {
              label: 'Building with Emscripten',
              to: 'docs/emscripten',
            },
          ],
        },
        {
          title: 'Integrations',
          items: [
            {
              label: 'Using with React Native',
              to: 'https://reactnative.dev/docs/hermes',
            },
            {
              label: 'Using a custom build with React Native',
              to: 'docs/react-native-integration',
            },
          ],
        },
        {
          title: 'More',
          items: [
            {
              label: 'Twitter',
              to: 'https://twitter.com/HermesEngine',
            },
            {
              label: 'GitHub',
              to: 'https://github.com/facebook/hermes',
            },
          ],
        },
        {
          title: 'Legal',
          // Please do not remove the privacy and terms, it's a legal requirement.
          items: [
            {
              label: 'Privacy',
              href: 'https://opensource.facebook.com/legal/privacy/',
            },
            {
              label: 'Terms',
              href: 'https://opensource.facebook.com/legal/terms/',
            },
            {
              label: 'Cookies',
              href: 'https://opensource.facebook.com/legal/cookie-policy/',
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
    },
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          showLastUpdateAuthor: false,
          showLastUpdateTime: true,
          editUrl: 'https://github.com/facebook/hermes/blob/HEAD/website/',
          path: '../doc',
          sidebarPath: require.resolve('./sidebars.json'),
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
  plugins: [
    path.join(__dirname, '/plugins/monaco-editor'),
    path.join(__dirname, '/plugins/case-sensitive-paths'),
  ],
};
