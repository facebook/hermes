# Hermes JS Engine Website

This directory contains the source code for the Hermes JS Engine website available at https://hermesengine.dev/.

## Developing

This project uses [Docusaurus v2](https://v2.docusaurus.io/).

To develop locally run `yarn` to install dependencies and then `yarn start` to start the website and begin development. Docusaurus will watch the files and automatically compile changes and update the browser preview.

For more information see the [Docusaurus website](https://v2.docusaurus.io/).

## Deploying

To deploy you must have push access.

We deploy the website to the [gh-pages](https://github.com/facebook/hermes/tree/gh-pages) branch of this repo. At a high level, the process of deploying is to build the website assets using Docusaurus, copy the built files over to the `gh-pages` branch (replacing everything that's there except `hermes.js` and `hermes.mem`), verify the changes, and push to the `gh-pages` branch.

The full steps are:
- `git checkout master`
- `cd website && yarn build`
- Copy the `website/build` directory
- `git checkout gh-pages`
- Delete everything except `hermes.js` and `hermes.mem`*
- Copy the `website/build` directory built above
- Run `npx simplehttpserver .` and open http://localhost:8000
- Verify the site is correct
- Verify all file changes are correct
- `git commit -m "Deploy website version based on <commit>"`
- `git push`

GitHub will automatically update the site within minutes.

### Updating Playground Hermes

In the above steps to deploy, we assume that you will not be compiling a new verison of Hermes for the playground with your deploy. If you would like to deploy a new version, then after building the site, you can run `./website/build-hermes` (Emscripten required). Then, continue with the remaining steps and replace both the `hermes.js` and `hermes.mem` files on the `gh-pages` branch with the files you built.
