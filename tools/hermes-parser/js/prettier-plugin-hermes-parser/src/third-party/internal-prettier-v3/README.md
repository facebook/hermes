# Modified prettier v3

This directory contains a modified version of prettier v3.

## Recreating prettier files

1. Check out prettier fork: `https://github.com/pieterv/prettier/tree/hermes-v2-backport`.
2. Build repo: `yarn build --no-minify`
3. Copy built files `dist/ast-to-doc.js` and all files in the `dist/plugins` directory to this location.
