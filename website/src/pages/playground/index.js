/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import React, { useEffect, useState, useReducer } from 'react';
import classnames from 'classnames';
import MonacoEditor from 'react-monaco-editor';
import Layout from '@theme/Layout';
import useTheme from './useTheme';
import Worker from 'worker-loader!./worker.js';
import styles from './styles.module.css';
import RunIcon from './runIcon';
import useWindowSize from './useWindowSize';

const vsDarkTheme = {
  base: 'vs-dark',
  inherit: true,
  rules: [{ background: '121212' }],
  colors: {
    'editor.background': '#121212',
  },
};

const worker = new Worker();

function reducer(state, action) {
  switch (action.type) {
    case 'request':
      return { ...state, loading: true, lastTime: new Date() };
    case 'log':
      return {
        ...state,
        output: state.output + action.log,
      };
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
  const [{ loading, ellapsed, output }, dispatch] = useReducer(
    reducer,
    initialState
  );
  const [input, setInput] = useState('const a = 1; \nprint(a);');
  const [args, setArgs] = useState('-help');
  const windowSize = useWindowSize();
  const theme = useTheme();

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
        case 'log':
          dispatch({ type: 'log', log: e.data[1].log });
          break;
        case 'result':
          dispatch({
            type: 'success',
            result: e.data[1].result,
          });
          break;
      }
    };
  }, []);

  function run() {
    if (loading) {
      return;
    }

    const opts = args.split(/\s+/).filter(x => x);
    dispatch({ type: 'request' });
    worker.postMessage(['run', opts, input]);
  }

  function handleSubmit(e) {
    e.preventDefault();
    run();
  }

  function handleArgsChange(evt) {
    setArgs(evt.target.value);
  }

  function onEditorWillMount(monaco) {
    monaco.editor.defineTheme('vs-dark', vsDarkTheme);
  }

  return (
    <Layout title="Hermes" description="Hermes Playground" noFooter={true}>
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
        <button onClick={run} className={classnames(styles.argsIconContainer)}>
          <RunIcon loading={loading} />
        </button>
        <span className={classnames(styles.ellapsed)}>{ellapsed}</span>
      </form>

      <div className={styles.row}>
        <MonacoEditor
          {...(windowSize.width > 600 ? editorLayout.lg : editorLayout.xs)}
          language="javascript"
          theme={theme === 'dark' ? 'vs-dark' : 'vs-light'}
          value={input}
          onChange={setInput}
          options={{ minimap: { enabled: false }, wordWrap: 'on' }}
          editorWillMount={onEditorWillMount}
        />
        <MonacoEditor
          {...(windowSize.width > 600 ? outputLayout.lg : outputLayout.xs)}
          language="json"
          theme={theme === 'dark' ? 'vs-dark' : 'vs-light'}
          value={output}
          options={{
            readOnly: true,
            minimap: { enabled: false },
            wordWrap: 'on',
          }}
          editorWillMount={onEditorWillMount}
        />
      </div>
    </Layout>
  );
}

export default Playground;
