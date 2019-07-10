/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

module.exports = {
  title: 'Hermes',
  tagline: 'Hermes is a JavaScript engine optimized for React Native.',
  url: 'https://hermesvm.io',
  baseUrl: '/',
  favicon: 'img/favicon.ico',
  themeConfig: {
    navbar: {
      title: 'Hermes',
      logo: {
        alt: 'Hermes Logo',
        src: 'img/logo.svg',
      },
      links: [
        { to: 'docs/', label: 'Docs', position: 'left' },
        { to: 'blog', label: 'Blog', position: 'left' },
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
              label: 'Coding Standards',
              to: 'docs/coding-standards',
            },
            {
              label: 'Design',
              to: 'docs/design',
            },
            {
              label: 'IR',
              to: 'docs/ir',
            },
            {
              label: 'Modules',
              to: 'docs/modules',
            },
            {
              label: 'Optimizer',
              to: 'docs/optimizer',
            },
            {
              label: 'VM',
              to: 'docs/vm',
            },
            {
              label: 'Strings',
              to: 'docs/strings',
            },
          ],
        },
        {
          title: 'Community',
          items: [
            {
              label: 'Discord',
              href: 'https://discordapp.com/invite/docusaurus',
            },
          ],
        },
        {
          title: 'Social',
          items: [
            {
              label: 'Blog',
              to: 'blog',
            },
          ],
        },
      ],
      logo: {
        alt: 'Facebook Open Source Logo',
        src: 'https://docusaurus.io/img/oss_logo.png',
      },
      copyright: `Copyright © ${new Date().getFullYear()} Facebook, Inc. Built with Docusaurus.`,
    },
  },
  presets: [
    [
      '@docusaurus/preset-classic',
      {
        docs: {
          sidebarPath: require.resolve('./sidebars.js'),
          path: '../doc'
        },
        pages: {
          path: 'src/pages',
        },
        theme: {
          customCss: require.resolve('./src/css/custom.css'),
        },
      },
    ],
  ],
};
