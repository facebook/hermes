# Modified prettier v3

This directory contains a modified version of prettier v3's Hermes plugin.

## Recreating prettier files

To make changes to our fork of prettier v3:

1. Run yarn build script: `./scripts/build-prettier`
3. Make any modifications to the prettier git repo inside `hermes-parser/js/prettier-hermes-flow-fork`
4. Run `yarn build-prettier` after making any changes.
5. Once you're done making changes, commit inside the prettier-hermes-flow-fork git repo, and push your changes to the [upstream repo](https://github.com/pieterv/prettier/tree/flow-fork)

### Manual process

1. Check out prettier fork: `https://github.com/pieterv/prettier/tree/flow-fork`.
2. Build repo: `yarn build --package=@prettier/plugin-hermes`
3. Copy built files `dist/plugin-hermes` and all JS files over there to this location.
