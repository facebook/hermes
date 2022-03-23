/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import React, {useEffect, useState, useReducer, useCallback} from 'react';
import Head from '@docusaurus/Head';
import Layout from '@theme/Layout';
import HermesWorker from 'worker-loader!@site/src/workers/hermesWorker.js';
import Code from '@site/src/components/Code';
import Spinner from '@site/src/components/Spinner';
import styles from './styles.module.css';

let hermesWorker;
if (typeof window !== 'undefined') {
  hermesWorker = new HermesWorker();
}

const initialState = {
  runRequested: false,
  output: '',
  lastTime: undefined,
  elapsed: 0,
};

function reducer(state, action) {
  switch (action.tag) {
    case 'RUN':
      if (state.runRequested) return state;
      const args = action.args.split(/\s+/).filter(x => x);
      hermesWorker.postMessage(['run', args, action.source]);
      return {...state, runRequested: true, lastTime: new Date()};
    case 'RUN_COMPLETE':
      return {
        ...state,
        runRequested: false,
        output: action.result,
        elapsed: new Date() - state.lastTime,
      };
  }
}

function Playground() {
  if (typeof window === 'undefined') return null;

  const [source, setSource] = useState('const a = 1; \nprint(a);');
  const [args, setArgs] = useState('-O -dump-bytecode');
  const [state, dispatch] = useReducer(reducer, initialState);

  const {runRequested, elapsed, output} = state;
  const run = args => dispatch({tag: 'RUN', args, source});

  useEffect(() => {
    hermesWorker.onmessage = e => {
      let [tag, payload] = e.data;
      switch (tag) {
        case 'runResult':
          dispatch({tag: 'RUN_COMPLETE', result: payload});
          break;
        default:
          throw new Error('unknown message type from Hermes worker.');
      }
    };
  }, []);

  // A hack to completely disasble scrolling behaviors on mobile
  // e.g. bounce effect on iOS, pull-to-refresh on Android.
  //
  // The reason to do it via effects on `html` is that docusaurus drawer
  // will mutate `body`'s `overflow` and we have no way to override it.
  useEffect(() => {
    document.querySelector('html').style.overflow = 'hidden';
    return () => {
      document.querySelector('html').style.overflow = 'visible';
    };
  });

  const handleSubmit = e => {
    e.preventDefault();
    run(args);
  };

  return (
    <Layout title="Hermes" description="Hermes Playground" noFooter={true}>
      <Head>
        <meta
          name="viewport"
          content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no"
        />
        <link rel="manifest" href="/manifest.webmanifest" />
      </Head>
      <form className={styles.driver} onSubmit={handleSubmit}>
        <input
          className={styles.argsInput}
          name="args"
          placeholder="args"
          type="text"
          value={args}
          onChange={e => setArgs(e.target.value)}
        />
        <button className={styles.runBtn}>
          <RunIcon isLoading={runRequested} />
        </button>
        <div className={styles.helpBtn} onClick={_ => run('-help')}>
          ?
        </div>
        <span className={styles.elapsed}>{elapsed} ms</span>
      </form>

      <div className={styles.codeContainer}>
        <div className={styles.sourceCode}>
          <Code language="javascript" value={source} onChange={setSource} />
        </div>
        <div className={styles.outputCode}>
          <Code
            language="json"
            value={output}
            editorDidMount={useCallback(_ => run(args))}
            options={{readOnly: true}}
          />
        </div>
      </div>
    </Layout>
  );
}

function RunIcon({isLoading}) {
  if (isLoading) return <Spinner />;
  return (
    <i aria-label="icon: run">
      <svg
        viewBox="0 0 1200 1200"
        data-icon="run"
        width="1.5em"
        height="1.5em"
        fill="currentColor"
        aria-hidden="true">
        <path d="M 600,1200 C 268.65,1200 0,931.35 0,600 0,268.65 268.65,0 600,0 c 331.35,0 600,268.65 600,600 0,331.35 -268.65,600 -600,600 z M 450,300.45 450,899.55 900,600 450,300.45 z" />
      </svg>
    </i>
  );
}

export default Playground;
