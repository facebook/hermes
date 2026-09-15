import path from "node:path";
import { test, type TestContext } from "node:test";

import { listDirectoryEntries, runFileInSubprocess } from "./tests.ts";

const ROOT_PATH = path.resolve(import.meta.dirname, "..", "..");
const TESTS_ROOT_PATH = path.join(ROOT_PATH, "tests");

const ASSERT_MODULE_PATH = path.join(
  ROOT_PATH,
  "implementors",
  "node",
  "assert.js"
);
const LOAD_ADDON_MODULE_PATH = path.join(
  ROOT_PATH,
  "implementors",
  "node",
  "load-addon.js"
);

async function populateSuite(
  testContext: TestContext,
  dir: string
): Promise<void> {
  const { directories, files } = listDirectoryEntries(dir);

  for (const file of files) {
    await testContext.test(file, () => runFileInSubprocess(dir, file));
  }

  for (const directory of directories) {
    await testContext.test(directory, async (subTest) => {
      await populateSuite(subTest, path.join(dir, directory));
    });
  }
}

test("harness", async (t) => {
  await populateSuite(t, path.join(TESTS_ROOT_PATH, "harness"));
});

test("js-native-api", async (t) => {
  await populateSuite(t, path.join(TESTS_ROOT_PATH, "js-native-api"));
});

test("node-api", async (t) => {
  await populateSuite(t, path.join(TESTS_ROOT_PATH, "node-api"));
});
