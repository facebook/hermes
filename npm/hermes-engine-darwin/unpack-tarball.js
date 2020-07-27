const path = require("path");
const fs = require("fs");
const child_process = require("child_process");

if (
  process.platform === "darwin" &&
  !fs.existsSync(path.join(__dirname, "destroot"))
) {
  const tarball = fs.readdirSync(__dirname).find(function (entry) {
    return /^hermes-runtime-darwin-v[\d\.]+\.tar\.gz$/.test(entry);
  });
  if (!tarball) {
    throw new Error("Could not locate tarball");
  }
  child_process.execFileSync("tar", ["-xzvf", path.join(__dirname, tarball)]);
}
