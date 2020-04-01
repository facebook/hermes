/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import React, { useEffect, useState, useReducer } from 'react';
import classnames from 'classnames';
import Layout from '@theme/Layout';
import Worker from 'worker-loader!@site/src/workers/hermes.js';
import useWindowSize from '@site/src/hooks/useWindowSize';
import Editor from '@site/src/components/Editor';
import styles from './styles.module.css';

function reducer(state, action) {
  switch (action.type) {
    case 'request':
      return { ...state, loading: true, lastTime: new Date() };
    case 'success':
      return {
        ...state,
        loading: false,
        output: action.result,
        ellapsed: new Date() - state.lastTime + ' ms',
      };
  }
}

const initialState = {
  loading: false,
  output: '',
  lastTime: undefined,
  ellapsed: undefined,
};

function Playground() {
  if (typeof window === 'undefined') {
    return null;
  }

  const worker = new Worker();

  const [{ loading, ellapsed, output }, dispatch] = useReducer(
    reducer,
    initialState
  );
  const [input, setInput] = useState('const a = 1; \nprint(a);');
  const [args, setArgs] = useState('-O -dump-bytecode');
  const windowSize = useWindowSize();

  const headerLayout = {
    height: 100,
  };

  const editorLayout = {
    xs: {
      width: windowSize.width,
      height: 200,
    },
    lg: {
      width: (1 / 3) * windowSize.width,
      height: windowSize.height - headerLayout.height,
    },
  };

  const outputLayout = {
    xs: {
      width: windowSize.width,
      height: windowSize.height - headerLayout.height - editorLayout.xs.height,
    },
    lg: {
      width: (2 / 3) * windowSize.width,
      height: windowSize.height,
    },
  };

  useEffect(() => {
    worker.onmessage = function(e) {
      switch (e.data[0]) {
        case 'result':
          dispatch({
            type: 'success',
            result: e.data[1].result,
          });
          break;
      }
    };

    run(args);
  }, []);

  function run(args) {
    if (loading) {
      return;
    }

    const opts = args.split(/\s+/).filter(x => x);
    dispatch({ type: 'request' });
    worker.postMessage(['run', opts, input]);
  }

  function handleSubmit(e) {
    e.preventDefault();
    run(args);
  }

  function handleArgsChange(evt) {
    setArgs(evt.target.value);
  }

  function handleClickRun() {
    run(args);
  }

  function handleClickHelp() {
    run('-help');
  }

  return (
    <Layout title="Hermes" description="Hermes Playground" noFooter={true}>
      <div className={classnames(styles.headerContainer)}>
        <form
          onSubmit={handleSubmit}
          className={classnames(styles.argsInputContainer)}
        >
          <input
            className={classnames(styles.argsInputField)}
            name="args"
            placeholder="args"
            type="text"
            value={args}
            onChange={handleArgsChange}
          />
          <button
            onClick={handleClickRun}
            className={classnames(styles.argsIconContainer)}
          >
            <RunIcon loading={loading} />
          </button>
        </form>

        <div>
          <button
            onClick={handleClickHelp}
            className={classnames(styles.helperButton)}
          >
            ?
          </button>
          <span>{ellapsed}</span>
        </div>
      </div>

      <div className={classnames(styles.row)}>
        <Editor
          {...(windowSize.width > 600 ? editorLayout.lg : editorLayout.xs)}
          language="javascript"
          value={input}
          onChange={setInput}
          options={{ minimap: { enabled: false }, wordWrap: 'on' }}
        />
        <Editor
          {...(windowSize.width > 600 ? outputLayout.lg : outputLayout.xs)}
          language="json"
          value={output}
          options={{
            readOnly: true,
            minimap: { enabled: false },
            wordWrap: 'on',
          }}
        />
      </div>
    </Layout>
  );
}

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

export default Playground;
