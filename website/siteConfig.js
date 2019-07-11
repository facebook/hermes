const {baseUrlPattern} = require('./utils');

const isBuilding = process.env.npm_lifecycle_script
  && process.env.npm_lifecycle_script === 'docusaurus-build'; 

const siteConfig = {
  // Keep `/` in watch mode.
  baseUrl: isBuilding ? baseUrlPattern : '/',
  // ...
};

module.exports = siteConfig;

