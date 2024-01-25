/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -O -gc-sanitize-handles=0 %s

'use strict';

// Do a bunch of random sized allocations to exercise many paths in the GC.

(function main(numTimes) {
  // Set the seed for the random number generator.
  // Set to undefined to let the system choose a seed.
  var seed = 1464194546;
  var realRandom = Math.random;
  Math.random = function random() {
    if (seed === undefined) {
      seed = (realRandom() * 2 ** 31) | 0;
      print('Letting Math.random choose the seed as: ' + seed);
    }
    // Use a LCG as a simple random number generator.
    seed = (seed * 1664525 + 1013904223) | 0;
    return Math.abs(seed) / 2 ** 31;
  };

  var Random = {
    int: function int(limit) {
      return Math.trunc(Math.random() * limit);
    },

    coinflip: function coinflip(p) {
      return Math.random() < p;
    },

    choice: function choice(array) {
      return array[Random.int(array.length)];
    },

    character: function character() {
      return String.fromCodePoint(Random.int(0x7f));
    },

    unicodeCharacter: function unicodeCharacter() {
      return String.fromCodePoint(Random.int(0x10fff));
    },

    stringWithSize: function stringWithSize(size, ascii) {
      var result = '';
      for (var i = 0; i < size; i++) {
        result += ascii ? Random.character() : Random.unicodeCharacter();
      }
      return result;
    },

    string: function string() {
      return Random.stringWithSize(Random.int(1000), true);
    },

    unicodeString: function unicodeString() {
      return Random.stringWithSize(Random.int(1000), false);
    },

    addProperty: function addProperty(obj, val) {
      obj[Random.string()] = val;
    },

    addUnicodeProperty: function addUnicodeProperty(obj, val) {
      obj[Random.unicodeString()] = val;
    },

    addIndexedProperty: function addIndexedProperty(obj, limit, val) {
      obj[Random.int(limit)] = val;
    },
  };

  print('Start');
  // CHECK-LABEL: Start
  // Add an object here to age it.
  var objects = [];
  var objectsAdded = 0;
  var objectsRemoved = 0;
  var numOldToYoungCreated = 0;
  var numOldToOldCreated = 0;
  for (var i = 0; i < numTimes; i++) {
    var constructor = Random.choice([Object, Array]);
    var obj = new constructor();
    var filler = Random.choice([Random.string, Random.int.bind(null, 100)]);
    var numASCIIProps = Random.int(10);
    var numUnicodeProps = Random.int(2);
    if (constructor === Object) {
      for (var j = 0; j < numASCIIProps; j++) {
        Random.addProperty(obj, filler());
      }
      for (var j = 0; j < numUnicodeProps; j++) {
        Random.addUnicodeProperty(obj, filler());
      }
    } else {
      for (var j = 0; j < numASCIIProps; j++) {
        obj[j] = filler();
      }
    }
    if (Random.coinflip(0.25)) {
      // Age some objects so they can reach the old gen.
      objects.push(obj);
      objectsAdded++;
    }
    if (objects.length && Random.coinflip(0.1)) {
      // Cleanup some old objects with a lower frequency to occasionally clear
      // garbage out of the OG.
      objects.pop();
      objectsRemoved++;
    }
    // Create some old-to-young pointers to test the write barrier.
    if (objects.length && Random.coinflip(0.5)) {
      // To create an old-to-young pointer, pick a random object from objects
      // and have it point to obj which is likely in the YG.
      // Just in case obj is in OG though, have it point to a newly created object as well.
      Random.choice(objects).ptr = obj;
      Random.choice(objects).ptr = {yg: true};
      numOldToYoungCreated++;
    }
    // Create some old-to-old pointers to test the snapshot write barrier.
    if (objects.length && Random.coinflip(0.5)) {
      Random.choice(objects).ptr = Random.choice(objects);
      numOldToOldCreated++;
    }
    if (i !== 0 && i % 10 == 0) {
      print(
        'Created ' +
          i +
          ' objects total, old gen size is ' +
          objects.length +
          '.',
      );
      print('\tObjects added: ' + objectsAdded);
      print('\tObjects removed: ' + objectsRemoved);
      print('\tOld to young pointers created: ' + numOldToYoungCreated);
      print('\tOld to old pointers created: ' + numOldToOldCreated);
      objectsAdded = 0;
      objectsRemoved = 0;
      numOldToYoungCreated = 0;
      numOldToOldCreated = 0;
    }
  }
  print(
    'Created ' + i + ' objects total, old gen size is ' + objects.length + '.',
  );
  print('\tObjects added: ' + objectsAdded);
  print('\tObjects removed: ' + objectsRemoved);
  print('\tOld to young pointers created: ' + numOldToYoungCreated);
  print('\tOld to old pointers created: ' + numOldToOldCreated);
  print('Finished');
  // CHECK-LABEL: Finished
})(250);
