/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import { useState, useEffect } from 'react';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';

function useTheme() {
  const {
    siteConfig: { themeConfig: { disableDarkMode } } = {},
  } = useDocusaurusContext();
  const [theme, setTheme] = useState(
    typeof document !== 'undefined'
      ? document.documentElement.getAttribute('data-theme')
      : ''
  );

  useEffect(() => {
    if (disableDarkMode) {
      return;
    }

    function handler(e) {
      if (e.target.className === 'react-toggle-screenreader-only') {
        setTheme(e.target.checked ? 'dark' : '');
      }
    }

    document.documentElement.addEventListener('change', handler);
    return () => {
      document.documentElement.removeEventListener('change', handler);
    };
  }, []);

  return theme;
}

export default useTheme;
