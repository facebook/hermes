import path from 'node:path';

export async function resolve(specifier, context, nextResolve) {
  const result = await nextResolve(specifier, context);
  const ext = path.extname(new URL(result.url).pathname);
  if (ext === '.ts') {
    return {
      ...result,
      format: 'module',
    };
  }
  return result;
}

export async function load(url, context, nextLoad) {
  const ext = path.extname(new URL(url).pathname);
  if (ext === '.ts') {
    return nextLoad(url, {
      ...context,
      format: 'module-typescript',
    });
  }
  return nextLoad(url, context);
}
