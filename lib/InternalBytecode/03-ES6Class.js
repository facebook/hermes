/* @nolint */ var initES6Class = function () {
  function defineClass(ctor, superClass) {
    ctor.prototype = Object.create(superClass && superClass.prototype, {
      constructor: {
        value: ctor,
        writable: true,
        configurable: true,
      },
    });

    Object.defineProperty(ctor, "prototype", {
      writable: false,
    });

    if (superClass) {
      Object.setPrototypeOf(ctor, superClass);
    }
  }

  function defineClassProperty(cls, methodName, getter, setter) {
    Object.defineProperty(cls.prototype, methodName, {
      enumerable: false,
      get: getter,
      set: setter,
    });
  }

  function defineStaticClassProperty(cls, methodName, getter, setter) {
    Object.defineProperty(cls, methodName, {
      enumerable: false,
      get: getter,
      set: setter,
    });
  }

  function defineClassMethod(cls, methodName, fn) {
    Object.defineProperty(cls.prototype, methodName, {
      enumerable: false,
      value: fn,
    });
  }

  function defineStaticClassMethod(cls, methodName, fn) {
    Object.defineProperty(cls, methodName, {
      enumerable: false,
      value: fn,
    });
  }

  globalThis.HermesES6Internal = {
    defineClass,
    defineClassProperty,
    defineStaticClassProperty,
    defineClassMethod,
    defineStaticClassMethod,
  };
};

if (HermesInternal?.hasES6Class?.()) {
  initES6Class();
}
