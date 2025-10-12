#!/usr/bin/env node
const path = require('path');
const fs = require('fs');
const https = require('https');
const {spawnSync} = require('child_process');

const log = (msg) => process.stdout.write(`${msg}\n`);

const REPO_ROOT = path.resolve(__dirname, '..', '..');
const BASE_DIR = process.env.HERMES_WS_DIR || path.join(REPO_ROOT, 'build', 'win_hermesc');
const ICU_URL = process.env.ICU_URL || 'https://github.com/unicode-org/icu/releases/download/release-64-2/icu4c-64_2-Win64-MSVC2017.zip';
const ICU_DIR = path.join(BASE_DIR, 'icu');
const DEPS_DIR = path.join(BASE_DIR, 'deps');
const WIN64_BIN_DIR = path.join(BASE_DIR, 'win64-bin');
const CMAKE_BUILD_DIR = path.join(BASE_DIR, 'build_release');

const PATH_KEY = Object.keys(process.env).find((key) => key.toLowerCase() === 'path') || 'PATH';
const extraPath = [process.env.CMAKE_DIR, process.env.MSBUILD_DIR].filter(Boolean);
if (extraPath.length) {
  const currentPath = process.env[PATH_KEY] || '';
  const additions = extraPath.join(path.delimiter);
  process.env[PATH_KEY] = additions + (currentPath ? `${path.delimiter}${currentPath}` : '');
  log(`Extended PATH with: ${additions}`);
}

function ensureDir(dir) {
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, {recursive: true});
  }
}

function ensureWorkspace() {
  [BASE_DIR, path.join(BASE_DIR, 'osx-bin'), ICU_DIR, DEPS_DIR, WIN64_BIN_DIR].forEach(ensureDir);
}

function downloadFile(url, dest) {
  return new Promise((resolve, reject) => {
    if (fs.existsSync(dest)) {
      fs.unlinkSync(dest);
    }
    const file = fs.createWriteStream(dest);
    https.get(url, (response) => {
      if (response.statusCode && response.statusCode >= 300 && response.statusCode < 400 && response.headers.location) {
        downloadFile(response.headers.location, dest).then(resolve).catch(reject);
        return;
      }
      if (response.statusCode !== 200) {
        reject(new Error(`Download failed with status ${response.statusCode}`));
        return;
      }
      response.pipe(file);
      file.on('finish', () => {
        file.close(resolve);
      });
    }).on('error', (err) => {
      fs.unlink(dest, () => reject(err));
    });
  });
}

function run(command, args, options = {}) {
  log(`> ${command} ${(args || []).join(' ')}`);
  const result = spawnSync(command, args, {
    stdio: 'inherit',
    shell: false,
    ...options,
  });
  if (result.error) {
    if (result.error.code === 'ENOENT') {
      throw new Error(`Command not found: ${command}. Update PATH or set CMAKE_DIR/MSBUILD_DIR.`);
    }
    throw new Error(`Failed to launch ${command}: ${result.error.message}`);
  }
  if (result.status !== 0) {
    throw new Error(`${command} exited with code ${result.status}`);
  }
}

function expandZip(zipPath, destination) {
  run('powershell.exe', [
    '-NoLogo',
    '-NoProfile',
    '-Command',
    `Expand-Archive -Path '${zipPath}' -DestinationPath '${destination}' -Force`,
  ]);
}

function copyFile(source, destination) {
  ensureDir(path.dirname(destination));
  fs.copyFileSync(source, destination);
}

function copyPattern(sourceDir, pattern, destinationDir) {
  const regex = new RegExp(pattern.replace(/\*/g, '.*'));
  for (const entry of fs.readdirSync(sourceDir)) {
    if (regex.test(entry)) {
      copyFile(path.join(sourceDir, entry), path.join(destinationDir, entry));
    }
  }
}

async function fetchIcu() {
  log('Downloading ICU');
  const zipPath = path.join(ICU_DIR, 'icu.zip');
  await downloadFile(ICU_URL, zipPath);
  log('Expanding ICU');
  expandZip(zipPath, ICU_DIR);
}

function copyRuntimeDependencies() {
  copyPattern(path.join(ICU_DIR, 'bin64'), '^icu.*\.dll$', DEPS_DIR);
  const system32 = path.join(process.env.windir || 'C:/Windows', 'System32');
  ['msvcp140.dll', 'vcruntime140.dll', 'vcruntime140_1.dll'].forEach((dll) => {
    const source = path.join(system32, dll);
    if (!fs.existsSync(source)) {
      throw new Error(`Missing runtime dependency: ${source}`);
    }
    copyFile(source, path.join(DEPS_DIR, dll));
  });
}

function ensureCMakeBuild() {
  const configureArgs = [
    '-S',
    '.',
    '-B',
    CMAKE_BUILD_DIR,
    '-G',
    'Visual Studio 17 2022',
    '-Ax64',
    '-DCMAKE_BUILD_TYPE=Release',
    '-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=True',
    '-DHERMES_ENABLE_WIN10_ICU_FALLBACK=OFF',
  ];
  run('cmake', configureArgs, {
    env: {
      ...process.env,
      ICU_ROOT: ICU_DIR,
    },
  });
  run('cmake', ['--build', CMAKE_BUILD_DIR, '--target', 'hermesc', '--config', 'Release'], {
    env: {
      ...process.env,
      BOOST_CONTEXT_MASM: 'OFF',
    },
  });
}

function stageArtifacts() {
  const hermescSource = path.join(CMAKE_BUILD_DIR, 'bin', 'Release', 'hermesc.exe');
  if (!fs.existsSync(hermescSource)) {
    throw new Error(`Build output missing: ${hermescSource}`);
  }
  copyFile(hermescSource, path.join(WIN64_BIN_DIR, 'hermesc.exe'));
  for (const entry of fs.readdirSync(DEPS_DIR)) {
    copyFile(path.join(DEPS_DIR, entry), path.join(WIN64_BIN_DIR, entry));
  }
}

async function main() {
  try {
    ensureWorkspace();
    await fetchIcu();
    copyRuntimeDependencies();
    ensureCMakeBuild();
    stageArtifacts();
    log('HermesC build artifacts staged to win64-bin');
  } catch (error) {
    log(`Build failed: ${error.message}`);
    process.exitCode = 1;
  }
}

main();
