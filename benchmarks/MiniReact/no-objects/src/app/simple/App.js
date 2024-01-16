/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {Props, React$MixedElement} from 'react';

import {useState} from 'react';

function Button(props: Props): React$MixedElement {
  return (
    <button id={props.id} onClick={props.onClick}>
      Click me
    </button>
  );
}

function Input(props: Props): React$MixedElement {
  return (
    <input
      id={props.id}
      type="text"
      onChange={props.onChange}
      value={props.value}
    />
  );
}

function TextArea(props: Props): React$MixedElement {
  return <textarea onChange={props.onChange}>{props.value}</textarea>;
}

function Select(props: Props): React$MixedElement {
  const children = [];
  for (let i = 0; i < props.options.length; i++) {
    const option = props.options[i];
    children.push(
      <option key={option.value} value={option.value}>
        {option.label}
      </option>,
    );
  }
  return <select onChange={props.onChange}>{children}</select>;
}

function Checkbox(props: Props): React$MixedElement {
  return (
    <input type="checkbox" checked={props.checked} onChange={props.onChange} />
  );
}

function Radio(props: Props): React$MixedElement {
  return (
    <input type="radio" checked={props.checked} onChange={props.onChange} />
  );
}

function Slider(props: Props): React$MixedElement {
  return (
    <input
      type="range"
      min={props.min}
      max={props.max}
      step={props.step}
      value={props.value}
      onChange={props.onChange}
    />
  );
}

function ProgressBar(props: Props): React$MixedElement {
  return <div style={{width: `${props.progress}%`}}></div>;
}

function Spinner(props: Props): React$MixedElement {
  return <div className="spinner">Loading...</div>;
}

function Modal(props: Props): React$MixedElement {
  if (!props.isOpen) {
    return <div className="modal closed" />;
  }

  return (
    <div className="modal open">
      <div className="overlay" onClick={props.onClose}>
        X
      </div>
      <div className="content">{props.children}</div>
    </div>
  );
}

function Tooltip(props: Props): React$MixedElement {
  if (!props.isOpen) {
    return <div className="tooltip closed" />;
  }

  return (
    <div className="tooltip open">
      <div className="arrow"></div>
      <div className="content">{props.children}</div>
    </div>
  );
}

export default function App(props: Props): React$MixedElement {
  const [text, setText] = useState<string>('');
  const [number, setNumber] = useState<number>(0);
  const [isChecked, setIsChecked] = useState<boolean>(false);
  const [isSelected, setIsSelected] = useState<boolean>(false);
  const [isOpen, setIsOpen] = useState<boolean>(false);
  const [isTooltipOpen, setIsTooltipOpen] = useState<boolean>(true);
  return (
    <div>
      <h1>React Benchmark</h1>
      <Button id="toggle-modal" onClick={(): void => setIsOpen(!isOpen)}>
        Toggle Modal
      </Button>
      <Modal isOpen={isOpen} onClose={(): void => setIsOpen(false)}>
        <h2>Modal Content</h2>
        <p>This is some modal content.</p>
        <Tooltip
          isOpen={isTooltipOpen}
          onClose={(): void => setIsTooltipOpen(false)}>
          <h3>Tooltip Content</h3>
          <p>This is some tooltip content.</p>
        </Tooltip>
      </Modal>
      <div>
        <h2>Form Elements</h2>
        <Input
          id="update-text"
          value={text}
          onChange={e => setText(e.target.value)}
        />
        <TextArea value={text} onChange={e => setText(e.target.value)} />
        <Select
          options={[
            {label: 'Option 1', value: 1},
            {label: 'Option 2', value: 2},
            {label: 'Option 3', value: 3},
          ]}
          onChange={e => setNumber(parseInt(e.target.value))}
        />
        <Checkbox
          checked={isChecked}
          onChange={e => setIsChecked(e.target.checked)}
        />
        <Radio
          checked={isSelected}
          onChange={e => setIsSelected(e.target.checked)}
        />
        <Slider
          min={0}
          max={100}
          step={1}
          value={number}
          onChange={e => setNumber(parseInt(e.target.value))}
        />
        <ProgressBar progress={number} />
        <Spinner />
      </div>
    </div>
  );
}
