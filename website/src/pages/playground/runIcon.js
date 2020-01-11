/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import React from 'react';
import classnames from 'classnames';
import styles from './styles.module.css';

function RunIcon({ loading }) {
  if (loading) {
    return (
      <i aria-label="icon: load">
        <svg
          viewBox="0 0 1024 1024"
          data-icon="loading"
          width="1.5em"
          height="1.5em"
          fill="currentColor"
          aria-hidden="true"
          className={classnames(styles.spinner)}
        >
          <path d="M988 548c-19.9 0-36-16.1-36-36 0-59.4-11.6-117-34.6-171.3a440.45 440.45 0 0 0-94.3-139.9 437.71 437.71 0 0 0-139.9-94.3C629 83.6 571.4 72 512 72c-19.9 0-36-16.1-36-36s16.1-36 36-36c69.1 0 136.2 13.5 199.3 40.3C772.3 66 827 103 874 150c47 47 83.9 101.8 109.7 162.7 26.7 63.1 40.2 130.2 40.2 199.3.1 19.9-16 36-35.9 36z"></path>
        </svg>
      </i>
    );
  }

  return (
    <i aria-label="icon: run">
      <svg
        viewBox="0 0 1200 1200"
        data-icon="run"
        width="1.5em"
        height="1.5em"
        fill="currentColor"
        aria-hidden="true"
      >
        <path d="M 600,1200 C 268.65,1200 0,931.35 0,600 0,268.65 268.65,0 600,0 c 331.35,0 600,268.65 600,600 0,331.35 -268.65,600 -600,600 z M 450,300.45 450,899.55 900,600 450,300.45 z" />
      </svg>
    </i>
  );
}

export default RunIcon;
