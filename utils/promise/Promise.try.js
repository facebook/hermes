Promise.try = function promiseTry(callbackFn) {
  var args = arguments;
  return new Promise(function (resolve) {
    resolve(callbackFn.apply(undefined, iterableToArray(args).slice(1)));
  });
};
