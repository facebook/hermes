Promise.withResolvers = function () {
  var resolve;
  var reject;

  var promise = new Promise(function (res, rej) {
    resolve = res;
    reject = rej;
  });

  return { promise: promise, resolve: resolve, reject: reject };
}
