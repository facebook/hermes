import React from 'react';
import MonacoEditor from 'react-monaco-editor';
import useThemeContext from '@theme/hooks/useThemeContext';

function Editor(props) {
  const { isDarkTheme } = useThemeContext();

  function onEditorWillMount(monaco) {
    const vsDarkTheme = {
      base: 'vs-dark',
      inherit: true,
      rules: [{ background: '121212' }],
      colors: {
        'editor.background': '#121212',
      },
    };

    monaco.editor.defineTheme('vs-dark', vsDarkTheme);

    if (props.editorWillMount) {
      props.editorWillMount(monaco);
    }
  }

  return (
    <MonacoEditor
      {...props}
      editorWillMount={onEditorWillMount}
      theme={isDarkTheme ? 'vs-dark' : 'vs-light'}
    />
  );
}

export default Editor;
