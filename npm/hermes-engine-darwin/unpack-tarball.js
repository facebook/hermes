const path = require("path");
const fs = require("fs");
const child_process = require("child_process");

if (
  process.platform === "darwin" &&
  !fs.existsSync(path.join(__dirname, "destroot"))
) {
  const tarball = fs.readdirSync(__dirname).find(function (entry) {
    return entry.endsWith(".tar.gz");
  });
  if (!tarball) {
    throw new Error("Could not locate tarball");
  }
  child_process.execFileSync("/usr/bin/tar", [
    "-xzvf",
    path.join(__dirname, tarball),
  ]);
}
