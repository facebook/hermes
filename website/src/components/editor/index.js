import React, {Suspense, lazy} from 'react';
import useThemeContext from '@theme/hooks/useThemeContext';

const MonacoEditor = lazy(() => import('react-monaco-editor'));

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
    <Suspense fallback={<div>Loading</div>}>
      <MonacoEditor
        {...props}
        editorWillMount={onEditorWillMount}
        theme={isDarkTheme ? 'vs-dark' : 'vs-light'}
      />
    </Suspense>
  );
}

export default Editor;