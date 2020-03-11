/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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
      links: [
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
              label: 'Using Hermes in a React Native app',
              to: 'https://reactnative.dev/docs/hermes',
            },
            {
              label: 'Hermes Development',
              to: 'https://github.com/facebook/hermes',
            },
          ],
        },
        {
          title: 'Social',
          items: [
            {
              label: 'Twitter',
              to: 'https://twitter.com/HermesEngine',
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
