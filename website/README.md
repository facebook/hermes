# Hermes JS Engine Website

This directory contains the source code for the Hermes JS Engine website available at https://hermesengine.dev/.

## Developing

This project uses [Docusaurus v2](https://v2.docusaurus.io/).

To develop locally run `yarn` to install dependencies and then `yarn start` to start the website and begin development. Docusaurus will watch the files and automatically compile changes and update the browser preview.

To develop the playground, see [the playground Hermes](#the-playground-hermes) section below.

For more information see the [Docusaurus website](https://v2.docusaurus.io/).

## Deploying

To deploy you must have push access.

We deploy the website to the [gh-pages](https://github.com/facebook/hermes/tree/gh-pages) branch of this repo. At a high level, the process of deploying is to build the website assets using Docusaurus, copy the built files over to the `gh-pages` branch (replacing everything that's there except `hermes.js` and `hermes.mem`), verify the changes, and push to the `gh-pages` branch.

The full steps are:
- `git checkout main`
- `cd website`
- Restore or update [the playground Hermes](#the-playground-hermes)
- Run `yarn build` to build the site
- Run `npm run serve` to preview the site
- Verify the site is correct
- Verify all file changes are correct
- Run `USE_SSH=true GIT_USER=<GITHUB_USERNAME> yarn deploy` to deploy

GitHub will automatically update the site within minutes.

### The Playground Hermes

The playground uses a Hermes WASM module [built with emscripten](https://hermesengine.dev/docs/emscripten).
The built artifacts (`hermes.js` and `hermes.wasm`) are intentionally not checked-in, but are required to
be present for developing the playground and deploying.

If you will not be compiling a new verison of Hermes for the playground with your deploy, simply download
the `hermes.js` and `hermes.wasm` from the [gh-pages branch](https://github.com/facebook/hermes/tree/gh-pages)
and copy them under the `static/`.

If you would like to deploy a new version, you can run `./website/build-hermes` with the required arguments
(see [building with emscripten](https://hermesengine.dev/docs/emscripten) for prerequisites) to compile from source. The script should automatically copy the built `hermes.js` and `hermes.wasm` to `static/`.
