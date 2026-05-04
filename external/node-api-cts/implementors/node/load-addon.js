import assert from "node:assert/strict";
import { dlopen } from "node:process";
import { constants } from "node:os";
import path from "node:path";
import fs from "node:fs";

const loadAddon = (addonFileName) => {
  assert(typeof addonFileName === "string", "Expected a string as addon filename");
  assert(!addonFileName.endsWith(".node"), "Expected addon filename without the .node extension");
  const addonPath = path.join(process.cwd(), addonFileName + ".node");
  assert(fs.existsSync(addonPath), `Expected ${addonPath} to exist - did you build the addons?`);
  const addon = { exports: {} };
  dlopen(addon, addonPath, constants.dlopen.RTLD_NOW);
  return addon.exports;
};

Object.assign(globalThis, { loadAddon });
