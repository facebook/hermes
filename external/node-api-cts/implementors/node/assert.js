
import { ok } from "node:assert/strict";

const assert = (value, message) => {
  ok(value, message);
};

Object.assign(globalThis, { assert });
