# Modified prettier v3

This directory contains a modified version of prettier v3.

## Recreating prettier files

To make changes to our fork of prettier v3:

1. Run yarn build script: `yarn build-prettier`
3. Make any modifications to the prettier git repo inside `hermes-parser/js/prettier-hermes-v2-backport`
4. Run `yarn build-prettier` after making any changes.
5. Once you're done making changes, commit inside the prettier-hermes-v2-backport git repo, and push your changes to the [upstream repo](https://github.com/pieterv/prettier/tree/hermes-v2-backport)

### Manual process

1. Check out prettier fork: `https://github.com/pieterv/prettier/tree/hermes-v2-backport`.
2. Build repo: `yarn build --no-minify`
3. Copy built files `dist/ast-to-doc.js` and all files in the `dist/plugins` directory to this location.
