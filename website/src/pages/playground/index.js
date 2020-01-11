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
import Worker from 'worker-loader!./worker.js';
import styles from './styles.module.css';
import RunIcon from './runIcon';
import useWindowSize from './useWindowSize';

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

  return (
    <Layout title="Hermes" description="Hermes Playground">
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
          width={windowSize.width / 2}
          height={windowSize.height - 300}
          language="javascript"
          theme="vs-light"
          value={input}
          onChange={setInput}
        />
        <MonacoEditor
          width={windowSize.width / 2}
          height={windowSize.height - 300}
          language="json"
          theme="vs-light"
          value={output}
          options={{ readOnly: true }}
        />
      </div>
    </Layout>
  );
}

export default Playground;
