// Copyright (C) 2021 Igalia, S.L. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
description: |
    This defines helper objects and functions for testing Temporal.
defines: [TemporalHelpers]
features: [Symbol.species, Symbol.iterator, Temporal]
---*/

const ASCII_IDENTIFIER = /^[$_a-zA-Z][$_a-zA-Z0-9]*$/u;

function formatPropertyName(propertyKey, objectName = "") {
  switch (typeof propertyKey) {
    case "symbol":
      if (Symbol.keyFor(propertyKey) !== undefined) {
        return `${objectName}[Symbol.for('${Symbol.keyFor(propertyKey)}')]`;
      } else if (propertyKey.description.startsWith('Symbol.')) {
        return `${objectName}[${propertyKey.description}]`;
      } else {
        return `${objectName}[Symbol('${propertyKey.description}')]`
      }
    case "string":
      if (propertyKey !== String(Number(propertyKey))) {
        if (ASCII_IDENTIFIER.test(propertyKey)) {
          return objectName ? `${objectName}.${propertyKey}` : propertyKey;
        }
        return `${objectName}['${propertyKey.replace(/'/g, "\\'")}']`
      }
      // fall through
    default:
      // integer or string integer-index
      return `${objectName}[${propertyKey}]`;
  }
}

const SKIP_SYMBOL = Symbol("Skip");

var TemporalHelpers = {
  /*
   * Codes and maximum lengths of months in the ISO 8601 calendar.
   */
  ISOMonths: [
    { month: 1, monthCode: "M01", daysInMonth: 31 },
    { month: 2, monthCode: "M02", daysInMonth: 29 },
    { month: 3, monthCode: "M03", daysInMonth: 31 },
    { month: 4, monthCode: "M04", daysInMonth: 30 },
    { month: 5, monthCode: "M05", daysInMonth: 31 },
    { month: 6, monthCode: "M06", daysInMonth: 30 },
    { month: 7, monthCode: "M07", daysInMonth: 31 },
    { month: 8, monthCode: "M08", daysInMonth: 31 },
    { month: 9, monthCode: "M09", daysInMonth: 30 },
    { month: 10, monthCode: "M10", daysInMonth: 31 },
    { month: 11, monthCode: "M11", daysInMonth: 30 },
    { month: 12, monthCode: "M12", daysInMonth: 31 }
  ],

  /*
   * assertDuration(duration, years, ...,  nanoseconds[, description]):
   *
   * Shorthand for asserting that each field of a Temporal.Duration is equal to
   * an expected value.
   */
  assertDuration(duration, years, months, weeks, days, hours, minutes, seconds, milliseconds, microseconds, nanoseconds, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(duration instanceof Temporal.Duration, `${prefix}instanceof`);
    assert.sameValue(duration.years, years, `${prefix}years result:`);
    assert.sameValue(duration.months, months, `${prefix}months result:`);
    assert.sameValue(duration.weeks, weeks, `${prefix}weeks result:`);
    assert.sameValue(duration.days, days, `${prefix}days result:`);
    assert.sameValue(duration.hours, hours, `${prefix}hours result:`);
    assert.sameValue(duration.minutes, minutes, `${prefix}minutes result:`);
    assert.sameValue(duration.seconds, seconds, `${prefix}seconds result:`);
    assert.sameValue(duration.milliseconds, milliseconds, `${prefix}milliseconds result:`);
    assert.sameValue(duration.microseconds, microseconds, `${prefix}microseconds result:`);
    assert.sameValue(duration.nanoseconds, nanoseconds, `${prefix}nanoseconds result`);
  },

  /*
   * assertDateDuration(duration, years, months, weeks, days, [, description]):
   *
   * Shorthand for asserting that each date field of a Temporal.Duration is
   * equal to an expected value.
   */
  assertDateDuration(duration, years, months, weeks, days, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(duration instanceof Temporal.Duration, `${prefix}instanceof`);
    assert.sameValue(duration.years, years, `${prefix}years result:`);
    assert.sameValue(duration.months, months, `${prefix}months result:`);
    assert.sameValue(duration.weeks, weeks, `${prefix}weeks result:`);
    assert.sameValue(duration.days, days, `${prefix}days result:`);
    assert.sameValue(duration.hours, 0, `${prefix}hours result should be zero:`);
    assert.sameValue(duration.minutes, 0, `${prefix}minutes result should be zero:`);
    assert.sameValue(duration.seconds, 0, `${prefix}seconds result should be zero:`);
    assert.sameValue(duration.milliseconds, 0, `${prefix}milliseconds result should be zero:`);
    assert.sameValue(duration.microseconds, 0, `${prefix}microseconds result should be zero:`);
    assert.sameValue(duration.nanoseconds, 0, `${prefix}nanoseconds result should be zero:`);
  },

  /*
   * assertDurationsEqual(actual, expected[, description]):
   *
   * Shorthand for asserting that each field of a Temporal.Duration is equal to
   * the corresponding field in another Temporal.Duration.
   */
  assertDurationsEqual(actual, expected, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(expected instanceof Temporal.Duration, `${prefix}expected value should be a Temporal.Duration`);
    TemporalHelpers.assertDuration(actual, expected.years, expected.months, expected.weeks, expected.days, expected.hours, expected.minutes, expected.seconds, expected.milliseconds, expected.microseconds, expected.nanoseconds, description);
  },

  /*
   * assertInstantsEqual(actual, expected[, description]):
   *
   * Shorthand for asserting that two Temporal.Instants are of the correct type
   * and equal according to their equals() methods.
   */
  assertInstantsEqual(actual, expected, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(expected instanceof Temporal.Instant, `${prefix}expected value should be a Temporal.Instant`);
    assert(actual instanceof Temporal.Instant, `${prefix}instanceof`);
    assert(actual.equals(expected), `${prefix}equals method`);
  },

  /*
   * assertPlainDate(date, year, ..., nanosecond[, description[, era, eraYear]]):
   *
   * Shorthand for asserting that each field of a Temporal.PlainDate is equal to
   * an expected value. (Except the `calendar` property, since callers may want
   * to assert either object equality with an object they put in there, or the
   * value of date.calendarId.)
   */
  assertPlainDate(date, year, month, monthCode, day, description = "", era = undefined, eraYear = undefined) {
    const prefix = description ? `${description}: ` : "";
    assert(date instanceof Temporal.PlainDate, `${prefix}instanceof`);
    assert.sameValue(date.era, era, `${prefix}era result:`);
    assert.sameValue(date.eraYear, eraYear, `${prefix}eraYear result:`);
    assert.sameValue(date.year, year, `${prefix}year result:`);
    assert.sameValue(date.month, month, `${prefix}month result:`);
    assert.sameValue(date.monthCode, monthCode, `${prefix}monthCode result:`);
    assert.sameValue(date.day, day, `${prefix}day result:`);
  },

  /*
   * assertPlainDateTime(datetime, year, ..., nanosecond[, description[, era, eraYear]]):
   *
   * Shorthand for asserting that each field of a Temporal.PlainDateTime is
   * equal to an expected value. (Except the `calendar` property, since callers
   * may want to assert either object equality with an object they put in there,
   * or the value of datetime.calendarId.)
   */
  assertPlainDateTime(datetime, year, month, monthCode, day, hour, minute, second, millisecond, microsecond, nanosecond, description = "", era = undefined, eraYear = undefined) {
    const prefix = description ? `${description}: ` : "";
    assert(datetime instanceof Temporal.PlainDateTime, `${prefix}instanceof`);
    assert.sameValue(datetime.era, era, `${prefix}era result:`);
    assert.sameValue(datetime.eraYear, eraYear, `${prefix}eraYear result:`);
    assert.sameValue(datetime.year, year, `${prefix}year result:`);
    assert.sameValue(datetime.month, month, `${prefix}month result:`);
    assert.sameValue(datetime.monthCode, monthCode, `${prefix}monthCode result:`);
    assert.sameValue(datetime.day, day, `${prefix}day result:`);
    assert.sameValue(datetime.hour, hour, `${prefix}hour result:`);
    assert.sameValue(datetime.minute, minute, `${prefix}minute result:`);
    assert.sameValue(datetime.second, second, `${prefix}second result:`);
    assert.sameValue(datetime.millisecond, millisecond, `${prefix}millisecond result:`);
    assert.sameValue(datetime.microsecond, microsecond, `${prefix}microsecond result:`);
    assert.sameValue(datetime.nanosecond, nanosecond, `${prefix}nanosecond result:`);
  },

  /*
   * assertPlainDateTimesEqual(actual, expected[, description]):
   *
   * Shorthand for asserting that two Temporal.PlainDateTimes are of the correct
   * type, equal according to their equals() methods, and additionally that
   * their calendar internal slots are the same value.
   */
  assertPlainDateTimesEqual(actual, expected, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(expected instanceof Temporal.PlainDateTime, `${prefix}expected value should be a Temporal.PlainDateTime`);
    assert(actual instanceof Temporal.PlainDateTime, `${prefix}instanceof`);
    assert(actual.equals(expected), `${prefix}equals method`);
    assert.sameValue(
      actual.getISOFields().calendar,
      expected.getISOFields().calendar,
      `${prefix}calendar same value:`
    );
  },

  /*
   * assertPlainMonthDay(monthDay, monthCode, day[, description [, referenceISOYear]]):
   *
   * Shorthand for asserting that each field of a Temporal.PlainMonthDay is
   * equal to an expected value. (Except the `calendar` property, since callers
   * may want to assert either object equality with an object they put in there,
   * or the value of monthDay.calendarId().)
   */
  assertPlainMonthDay(monthDay, monthCode, day, description = "", referenceISOYear = 1972) {
    const prefix = description ? `${description}: ` : "";
    assert(monthDay instanceof Temporal.PlainMonthDay, `${prefix}instanceof`);
    assert.sameValue(monthDay.monthCode, monthCode, `${prefix}monthCode result:`);
    assert.sameValue(monthDay.day, day, `${prefix}day result:`);
    assert.sameValue(monthDay.getISOFields().isoYear, referenceISOYear, `${prefix}referenceISOYear result:`);
  },

  /*
   * assertPlainTime(time, hour, ..., nanosecond[, description]):
   *
   * Shorthand for asserting that each field of a Temporal.PlainTime is equal to
   * an expected value.
   */
  assertPlainTime(time, hour, minute, second, millisecond, microsecond, nanosecond, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(time instanceof Temporal.PlainTime, `${prefix}instanceof`);
    assert.sameValue(time.hour, hour, `${prefix}hour result:`);
    assert.sameValue(time.minute, minute, `${prefix}minute result:`);
    assert.sameValue(time.second, second, `${prefix}second result:`);
    assert.sameValue(time.millisecond, millisecond, `${prefix}millisecond result:`);
    assert.sameValue(time.microsecond, microsecond, `${prefix}microsecond result:`);
    assert.sameValue(time.nanosecond, nanosecond, `${prefix}nanosecond result:`);
  },

  /*
   * assertPlainTimesEqual(actual, expected[, description]):
   *
   * Shorthand for asserting that two Temporal.PlainTimes are of the correct
   * type and equal according to their equals() methods.
   */
  assertPlainTimesEqual(actual, expected, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(expected instanceof Temporal.PlainTime, `${prefix}expected value should be a Temporal.PlainTime`);
    assert(actual instanceof Temporal.PlainTime, `${prefix}instanceof`);
    assert(actual.equals(expected), `${prefix}equals method`);
  },

  /*
   * assertPlainYearMonth(yearMonth, year, month, monthCode[, description[, era, eraYear, referenceISODay]]):
   *
   * Shorthand for asserting that each field of a Temporal.PlainYearMonth is
   * equal to an expected value. (Except the `calendar` property, since callers
   * may want to assert either object equality with an object they put in there,
   * or the value of yearMonth.calendarId.)
   */
  assertPlainYearMonth(yearMonth, year, month, monthCode, description = "", era = undefined, eraYear = undefined, referenceISODay = 1) {
    const prefix = description ? `${description}: ` : "";
    assert(yearMonth instanceof Temporal.PlainYearMonth, `${prefix}instanceof`);
    assert.sameValue(yearMonth.era, era, `${prefix}era result:`);
    assert.sameValue(yearMonth.eraYear, eraYear, `${prefix}eraYear result:`);
    assert.sameValue(yearMonth.year, year, `${prefix}year result:`);
    assert.sameValue(yearMonth.month, month, `${prefix}month result:`);
    assert.sameValue(yearMonth.monthCode, monthCode, `${prefix}monthCode result:`);
    assert.sameValue(yearMonth.getISOFields().isoDay, referenceISODay, `${prefix}referenceISODay result:`);
  },

  /*
   * assertZonedDateTimesEqual(actual, expected[, description]):
   *
   * Shorthand for asserting that two Temporal.ZonedDateTimes are of the correct
   * type, equal according to their equals() methods, and additionally that
   * their time zones and calendar internal slots are the same value.
   */
  assertZonedDateTimesEqual(actual, expected, description = "") {
    const prefix = description ? `${description}: ` : "";
    assert(expected instanceof Temporal.ZonedDateTime, `${prefix}expected value should be a Temporal.ZonedDateTime`);
    assert(actual instanceof Temporal.ZonedDateTime, `${prefix}instanceof`);
    assert(actual.equals(expected), `${prefix}equals method`);
    assert.sameValue(actual.timeZone, expected.timeZone, `${prefix}time zone same value:`);
    assert.sameValue(
      actual.getISOFields().calendar,
      expected.getISOFields().calendar,
      `${prefix}calendar same value:`
    );
  },

  /*
   * assertUnreachable(description):
   *
   * Helper for asserting that code is not executed. This is useful for
   * assertions that methods of user calendars and time zones are not called.
   */
  assertUnreachable(description) {
    let message = "This code should not be executed";
    if (description) {
      message = `${message}: ${description}`;
    }
    throw new Test262Error(message);
  },

  /*
   * checkCalendarDateUntilLargestUnitSingular(func, expectedLargestUnitCalls):
   *
   * When an options object with a largestUnit property is synthesized inside
   * Temporal and passed to user code such as calendar.dateUntil(), the value of
   * the largestUnit property should be in the singular form, even if the input
   * was given in the plural form.
   * (This doesn't apply when the options object is passed through verbatim.)
   *
   * func(calendar, largestUnit, index) is the operation under test. It's called
   * with an instance of a calendar that keeps track of which largestUnit is
   * passed to dateUntil(), each key of expectedLargestUnitCalls in turn, and
   * the key's numerical index in case the function needs to generate test data
   * based on the index. At the end, the actual values passed to dateUntil() are
   * compared with the array values of expectedLargestUnitCalls.
   */
  checkCalendarDateUntilLargestUnitSingular(func, expectedLargestUnitCalls) {
    const actual = [];

    class DateUntilOptionsCalendar extends Temporal.Calendar {
      constructor() {
        super("iso8601");
      }

      dateUntil(earlier, later, options) {
        actual.push(options.largestUnit);
        return super.dateUntil(earlier, later, options);
      }

      toString() {
        return "date-until-options";
      }
    }

    const calendar = new DateUntilOptionsCalendar();
    Object.entries(expectedLargestUnitCalls).forEach(([largestUnit, expected], index) => {
      func(calendar, largestUnit, index);
      assert.compareArray(actual, expected, `largestUnit passed to calendar.dateUntil() for largestUnit ${largestUnit}`);
      actual.splice(0); // empty it for the next check
    });
  },

  /*
   * checkPlainDateTimeConversionFastPath(func):
   *
   * ToTemporalDate and ToTemporalTime should both, if given a
   * Temporal.PlainDateTime instance, convert to the desired type by reading the
   * PlainDateTime's internal slots, rather than calling any getters.
   *
   * func(datetime, calendar) is the actual operation to test, that must
   * internally call the abstract operation ToTemporalDate or ToTemporalTime.
   * It is passed a Temporal.PlainDateTime instance, as well as the instance's
   * calendar object (so that it doesn't have to call the calendar getter itself
   * if it wants to make any assertions about the calendar.)
   */
  checkPlainDateTimeConversionFastPath(func, message = "checkPlainDateTimeConversionFastPath") {
    const actual = [];
    const expected = [];

    const calendar = new Temporal.Calendar("iso8601");
    const datetime = new Temporal.PlainDateTime(2000, 5, 2, 12, 34, 56, 987, 654, 321, calendar);
    const prototypeDescrs = Object.getOwnPropertyDescriptors(Temporal.PlainDateTime.prototype);
    ["year", "month", "monthCode", "day", "hour", "minute", "second", "millisecond", "microsecond", "nanosecond"].forEach((property) => {
      Object.defineProperty(datetime, property, {
        get() {
          actual.push(`get ${formatPropertyName(property)}`);
          const value = prototypeDescrs[property].get.call(this);
          return {
            toString() {
              actual.push(`toString ${formatPropertyName(property)}`);
              return value.toString();
            },
            valueOf() {
              actual.push(`valueOf ${formatPropertyName(property)}`);
              return value;
            },
          };
        },
      });
    });
    Object.defineProperty(datetime, "calendar", {
      get() {
        actual.push("get calendar");
        return calendar;
      },
    });

    func(datetime, calendar);
    assert.compareArray(actual, expected, `${message}: property getters not called`);
  },

  /*
   * Check that an options bag that accepts units written in the singular form,
   * also accepts the same units written in the plural form.
   * func(unit) should call the method with the appropriate options bag
   * containing unit as a value. This will be called twice for each element of
   * validSingularUnits, once with singular and once with plural, and the
   * results of each pair should be the same (whether a Temporal object or a
   * primitive value.)
   */
  checkPluralUnitsAccepted(func, validSingularUnits) {
    const plurals = {
      year: 'years',
      month: 'months',
      week: 'weeks',
      day: 'days',
      hour: 'hours',
      minute: 'minutes',
      second: 'seconds',
      millisecond: 'milliseconds',
      microsecond: 'microseconds',
      nanosecond: 'nanoseconds',
    };

    validSingularUnits.forEach((unit) => {
      const singularValue = func(unit);
      const pluralValue = func(plurals[unit]);
      const desc = `Plural ${plurals[unit]} produces the same result as singular ${unit}`;
      if (singularValue instanceof Temporal.Duration) {
        TemporalHelpers.assertDurationsEqual(pluralValue, singularValue, desc);
      } else if (singularValue instanceof Temporal.Instant) {
        TemporalHelpers.assertInstantsEqual(pluralValue, singularValue, desc);
      } else if (singularValue instanceof Temporal.PlainDateTime) {
        TemporalHelpers.assertPlainDateTimesEqual(pluralValue, singularValue, desc);
      } else if (singularValue instanceof Temporal.PlainTime) {
        TemporalHelpers.assertPlainTimesEqual(pluralValue, singularValue, desc);
      } else if (singularValue instanceof Temporal.ZonedDateTime) {
        TemporalHelpers.assertZonedDateTimesEqual(pluralValue, singularValue, desc);
      } else {
        assert.sameValue(pluralValue, singularValue);
      }
    });
  },

  /*
   * checkRoundingIncrementOptionWrongType(checkFunc, assertTrueResultFunc, assertObjectResultFunc):
   *
   * Checks the type handling of the roundingIncrement option.
   * checkFunc(roundingIncrement) is a function which takes the value of
   * roundingIncrement to test, and calls the method under test with it,
   * returning the result. assertTrueResultFunc(result, description) should
   * assert that result is the expected result with roundingIncrement: true, and
   * assertObjectResultFunc(result, description) should assert that result is
   * the expected result with roundingIncrement being an object with a valueOf()
   * method.
   */
  checkRoundingIncrementOptionWrongType(checkFunc, assertTrueResultFunc, assertObjectResultFunc) {
    // null converts to 0, which is out of range
    assert.throws(RangeError, () => checkFunc(null), "null");
    // Booleans convert to either 0 or 1, and 1 is allowed
    const trueResult = checkFunc(true);
    assertTrueResultFunc(trueResult, "true");
    assert.throws(RangeError, () => checkFunc(false), "false");
    // Symbols and BigInts cannot convert to numbers
    assert.throws(TypeError, () => checkFunc(Symbol()), "symbol");
    assert.throws(TypeError, () => checkFunc(2n), "bigint");

    // Objects prefer their valueOf() methods when converting to a number
    assert.throws(RangeError, () => checkFunc({}), "plain object");

    const expected = [
      "get roundingIncrement.valueOf",
      "call roundingIncrement.valueOf",
    ];
    const actual = [];
    const observer = TemporalHelpers.toPrimitiveObserver(actual, 2, "roundingIncrement");
    const objectResult = checkFunc(observer);
    assertObjectResultFunc(objectResult, "object with valueOf");
    assert.compareArray(actual, expected, "order of operations");
  },

  /*
   * checkStringOptionWrongType(propertyName, value, checkFunc, assertFunc):
   *
   * Checks the type handling of a string option, of which there are several in
   * Temporal.
   * propertyName is the name of the option, and value is the value that
   * assertFunc should expect it to have.
   * checkFunc(value) is a function which takes the value of the option to test,
   * and calls the method under test with it, returning the result.
   * assertFunc(result, description) should assert that result is the expected
   * result with the option value being an object with a toString() method
   * which returns the given value.
   */
  checkStringOptionWrongType(propertyName, value, checkFunc, assertFunc) {
    // null converts to the string "null", which is an invalid string value
    assert.throws(RangeError, () => checkFunc(null), "null");
    // Booleans convert to the strings "true" or "false", which are invalid
    assert.throws(RangeError, () => checkFunc(true), "true");
    assert.throws(RangeError, () => checkFunc(false), "false");
    // Symbols cannot convert to strings
    assert.throws(TypeError, () => checkFunc(Symbol()), "symbol");
    // Numbers convert to strings which are invalid
    assert.throws(RangeError, () => checkFunc(2), "number");
    // BigInts convert to strings which are invalid
    assert.throws(RangeError, () => checkFunc(2n), "bigint");

    // Objects prefer their toString() methods when converting to a string
    assert.throws(RangeError, () => checkFunc({}), "plain object");

    const expected = [
      `get ${propertyName}.toString`,
      `call ${propertyName}.toString`,
    ];
    const actual = [];
    const observer = TemporalHelpers.toPrimitiveObserver(actual, value, propertyName);
    const result = checkFunc(observer);
    assertFunc(result, "object with toString");
    assert.compareArray(actual, expected, "order of operations");
  },

  /*
   * checkSubclassingIgnored(construct, constructArgs, method, methodArgs,
   *   resultAssertions):
   *
   * Methods of Temporal classes that return a new instance of the same class,
   * must not take the constructor of a subclass into account, nor the @@species
   * property. This helper runs tests to ensure this.
   *
   * construct(...constructArgs) must yield a valid instance of the Temporal
   * class. instance[method](...methodArgs) is the method call under test, which
   * must also yield a valid instance of the same Temporal class, not a
   * subclass. See below for the individual tests that this runs.
   * resultAssertions() is a function that performs additional assertions on the
   * instance returned by the method under test.
   */
  checkSubclassingIgnored(...args) {
    this.checkSubclassConstructorNotObject(...args);
    this.checkSubclassConstructorUndefined(...args);
    this.checkSubclassConstructorThrows(...args);
    this.checkSubclassConstructorNotCalled(...args);
    this.checkSubclassSpeciesInvalidResult(...args);
    this.checkSubclassSpeciesNotAConstructor(...args);
    this.checkSubclassSpeciesNull(...args);
    this.checkSubclassSpeciesUndefined(...args);
    this.checkSubclassSpeciesThrows(...args);
  },

  /*
   * Checks that replacing the 'constructor' property of the instance with
   * various primitive values does not affect the returned new instance.
   */
  checkSubclassConstructorNotObject(construct, constructArgs, method, methodArgs, resultAssertions) {
    function check(value, description) {
      const instance = new construct(...constructArgs);
      instance.constructor = value;
      const result = instance[method](...methodArgs);
      assert.sameValue(Object.getPrototypeOf(result), construct.prototype, description);
      resultAssertions(result);
    }

    check(null, "null");
    check(true, "true");
    check("test", "string");
    check(Symbol(), "Symbol");
    check(7, "number");
    check(7n, "bigint");
  },

  /*
   * Checks that replacing the 'constructor' property of the subclass with
   * undefined does not affect the returned new instance.
   */
  checkSubclassConstructorUndefined(construct, constructArgs, method, methodArgs, resultAssertions) {
    let called = 0;

    class MySubclass extends construct {
      constructor() {
        ++called;
        super(...constructArgs);
      }
    }

    const instance = new MySubclass();
    assert.sameValue(called, 1);

    MySubclass.prototype.constructor = undefined;

    const result = instance[method](...methodArgs);
    assert.sameValue(called, 1);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
    resultAssertions(result);
  },

  /*
   * Checks that making the 'constructor' property of the instance throw when
   * called does not affect the returned new instance.
   */
  checkSubclassConstructorThrows(construct, constructArgs, method, methodArgs, resultAssertions) {
    function CustomError() {}
    const instance = new construct(...constructArgs);
    Object.defineProperty(instance, "constructor", {
      get() {
        throw new CustomError();
      }
    });
    const result = instance[method](...methodArgs);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
    resultAssertions(result);
  },

  /*
   * Checks that when subclassing, the subclass constructor is not called by
   * the method under test.
   */
  checkSubclassConstructorNotCalled(construct, constructArgs, method, methodArgs, resultAssertions) {
    let called = 0;

    class MySubclass extends construct {
      constructor() {
        ++called;
        super(...constructArgs);
      }
    }

    const instance = new MySubclass();
    assert.sameValue(called, 1);

    const result = instance[method](...methodArgs);
    assert.sameValue(called, 1);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
    resultAssertions(result);
  },

  /*
   * Check that the constructor's @@species property is ignored when it's a
   * constructor that returns a non-object value.
   */
  checkSubclassSpeciesInvalidResult(construct, constructArgs, method, methodArgs, resultAssertions) {
    function check(value, description) {
      const instance = new construct(...constructArgs);
      instance.constructor = {
        [Symbol.species]: function() {
          return value;
        },
      };
      const result = instance[method](...methodArgs);
      assert.sameValue(Object.getPrototypeOf(result), construct.prototype, description);
      resultAssertions(result);
    }

    check(undefined, "undefined");
    check(null, "null");
    check(true, "true");
    check("test", "string");
    check(Symbol(), "Symbol");
    check(7, "number");
    check(7n, "bigint");
    check({}, "plain object");
  },

  /*
   * Check that the constructor's @@species property is ignored when it's not a
   * constructor.
   */
  checkSubclassSpeciesNotAConstructor(construct, constructArgs, method, methodArgs, resultAssertions) {
    function check(value, description) {
      const instance = new construct(...constructArgs);
      instance.constructor = {
        [Symbol.species]: value,
      };
      const result = instance[method](...methodArgs);
      assert.sameValue(Object.getPrototypeOf(result), construct.prototype, description);
      resultAssertions(result);
    }

    check(true, "true");
    check("test", "string");
    check(Symbol(), "Symbol");
    check(7, "number");
    check(7n, "bigint");
    check({}, "plain object");
  },

  /*
   * Check that the constructor's @@species property is ignored when it's null.
   */
  checkSubclassSpeciesNull(construct, constructArgs, method, methodArgs, resultAssertions) {
    let called = 0;

    class MySubclass extends construct {
      constructor() {
        ++called;
        super(...constructArgs);
      }
    }

    const instance = new MySubclass();
    assert.sameValue(called, 1);

    MySubclass.prototype.constructor = {
      [Symbol.species]: null,
    };

    const result = instance[method](...methodArgs);
    assert.sameValue(called, 1);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
    resultAssertions(result);
  },

  /*
   * Check that the constructor's @@species property is ignored when it's
   * undefined.
   */
  checkSubclassSpeciesUndefined(construct, constructArgs, method, methodArgs, resultAssertions) {
    let called = 0;

    class MySubclass extends construct {
      constructor() {
        ++called;
        super(...constructArgs);
      }
    }

    const instance = new MySubclass();
    assert.sameValue(called, 1);

    MySubclass.prototype.constructor = {
      [Symbol.species]: undefined,
    };

    const result = instance[method](...methodArgs);
    assert.sameValue(called, 1);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
    resultAssertions(result);
  },

  /*
   * Check that the constructor's @@species property is ignored when it throws,
   * i.e. it is not called at all.
   */
  checkSubclassSpeciesThrows(construct, constructArgs, method, methodArgs, resultAssertions) {
    function CustomError() {}

    const instance = new construct(...constructArgs);
    instance.constructor = {
      get [Symbol.species]() {
        throw new CustomError();
      },
    };

    const result = instance[method](...methodArgs);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
  },

  /*
   * checkSubclassingIgnoredStatic(construct, method, methodArgs, resultAssertions):
   *
   * Static methods of Temporal classes that return a new instance of the class,
   * must not use the this-value as a constructor. This helper runs tests to
   * ensure this.
   *
   * construct[method](...methodArgs) is the static method call under test, and
   * must yield a valid instance of the Temporal class, not a subclass. See
   * below for the individual tests that this runs.
   * resultAssertions() is a function that performs additional assertions on the
   * instance returned by the method under test.
   */
  checkSubclassingIgnoredStatic(...args) {
    this.checkStaticInvalidReceiver(...args);
    this.checkStaticReceiverNotCalled(...args);
    this.checkThisValueNotCalled(...args);
  },

  /*
   * Check that calling the static method with a receiver that's not callable,
   * still calls the intrinsic constructor.
   */
  checkStaticInvalidReceiver(construct, method, methodArgs, resultAssertions) {
    function check(value, description) {
      const result = construct[method].apply(value, methodArgs);
      assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
      resultAssertions(result);
    }

    check(undefined, "undefined");
    check(null, "null");
    check(true, "true");
    check("test", "string");
    check(Symbol(), "symbol");
    check(7, "number");
    check(7n, "bigint");
    check({}, "Non-callable object");
  },

  /*
   * Check that calling the static method with a receiver that returns a value
   * that's not callable, still calls the intrinsic constructor.
   */
  checkStaticReceiverNotCalled(construct, method, methodArgs, resultAssertions) {
    function check(value, description) {
      const receiver = function () {
        return value;
      };
      const result = construct[method].apply(receiver, methodArgs);
      assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
      resultAssertions(result);
    }

    check(undefined, "undefined");
    check(null, "null");
    check(true, "true");
    check("test", "string");
    check(Symbol(), "symbol");
    check(7, "number");
    check(7n, "bigint");
    check({}, "Non-callable object");
  },

  /*
   * Check that the receiver isn't called.
   */
  checkThisValueNotCalled(construct, method, methodArgs, resultAssertions) {
    let called = false;

    class MySubclass extends construct {
      constructor(...args) {
        called = true;
        super(...args);
      }
    }

    const result = MySubclass[method](...methodArgs);
    assert.sameValue(called, false);
    assert.sameValue(Object.getPrototypeOf(result), construct.prototype);
    resultAssertions(result);
  },

  /*
   * Check that any iterable returned from a custom time zone's
   * getPossibleInstantsFor() method is exhausted.
   * The custom time zone object is passed in to func().
   * expected is an array of strings representing the expected calls to the
   * getPossibleInstantsFor() method. The PlainDateTimes that it is called with,
   * are compared (using their toString() results) with the array.
   */
  checkTimeZonePossibleInstantsIterable(func, expected) {
    // A custom time zone that returns an iterable instead of an array from its
    // getPossibleInstantsFor() method, and for testing purposes skips
    // 00:00-01:00 UTC on January 1, 2030, and repeats 00:00-01:00 UTC+1 on
    // January 3, 2030. Otherwise identical to the UTC time zone.
    class TimeZonePossibleInstantsIterable extends Temporal.TimeZone {
      constructor() {
        super("UTC");
        this.getPossibleInstantsForCallCount = 0;
        this.getPossibleInstantsForCalledWith = [];
        this.getPossibleInstantsForReturns = [];
        this.iteratorExhausted = [];
      }

      toString() {
        return "Custom/Iterable";
      }

      getOffsetNanosecondsFor(instant) {
        if (Temporal.Instant.compare(instant, "2030-01-01T00:00Z") >= 0 &&
          Temporal.Instant.compare(instant, "2030-01-03T01:00Z") < 0) {
          return 3600_000_000_000;
        } else {
          return 0;
        }
      }

      getPossibleInstantsFor(dateTime) {
        this.getPossibleInstantsForCallCount++;
        this.getPossibleInstantsForCalledWith.push(dateTime);

        // Fake DST transition
        let retval = super.getPossibleInstantsFor(dateTime);
        if (dateTime.toPlainDate().equals("2030-01-01") && dateTime.hour === 0) {
          retval = [];
        } else if (dateTime.toPlainDate().equals("2030-01-03") && dateTime.hour === 0) {
          retval.push(retval[0].subtract({ hours: 1 }));
        } else if (dateTime.year === 2030 && dateTime.month === 1 && dateTime.day >= 1 && dateTime.day <= 2) {
          retval[0] = retval[0].subtract({ hours: 1 });
        }

        this.getPossibleInstantsForReturns.push(retval);
        this.iteratorExhausted.push(false);
        return {
          callIndex: this.getPossibleInstantsForCallCount - 1,
          timeZone: this,
          *[Symbol.iterator]() {
            yield* this.timeZone.getPossibleInstantsForReturns[this.callIndex];
            this.timeZone.iteratorExhausted[this.callIndex] = true;
          },
        };
      }
    }

    const timeZone = new TimeZonePossibleInstantsIterable();
    func(timeZone);

    assert.sameValue(timeZone.getPossibleInstantsForCallCount, expected.length, "getPossibleInstantsFor() method called correct number of times");

    for (let index = 0; index < expected.length; index++) {
      assert.sameValue(timeZone.getPossibleInstantsForCalledWith[index].toString(), expected[index], "getPossibleInstantsFor() called with expected PlainDateTime");
      assert(timeZone.iteratorExhausted[index], "iterated through the whole iterable");
    }
  },

  /*
   * Check that any calendar-carrying Temporal object has its [[Calendar]]
   * internal slot read by ToTemporalCalendar, and does not fetch the calendar
   * by calling getters.
   * The custom calendar object is passed in to func() so that it can do its
   * own additional assertions involving the calendar if necessary. (Sometimes
   * there is nothing to assert as the calendar isn't stored anywhere that can
   * be asserted about.)
   */
  checkToTemporalCalendarFastPath(func) {
    class CalendarFastPathCheck extends Temporal.Calendar {
      constructor() {
        super("iso8601");
      }

      dateFromFields(...args) {
        return super.dateFromFields(...args).withCalendar(this);
      }

      monthDayFromFields(...args) {
        const { isoYear, isoMonth, isoDay } = super.monthDayFromFields(...args).getISOFields();
        return new Temporal.PlainMonthDay(isoMonth, isoDay, this, isoYear);
      }

      yearMonthFromFields(...args) {
        const { isoYear, isoMonth, isoDay } = super.yearMonthFromFields(...args).getISOFields();
        return new Temporal.PlainYearMonth(isoYear, isoMonth, this, isoDay);
      }

      toString() {
        return "fast-path-check";
      }
    }
    const calendar = new CalendarFastPathCheck();

    const plainDate = new Temporal.PlainDate(2000, 5, 2, calendar);
    const plainDateTime = new Temporal.PlainDateTime(2000, 5, 2, 12, 34, 56, 987, 654, 321, calendar);
    const plainMonthDay = new Temporal.PlainMonthDay(5, 2, calendar);
    const plainYearMonth = new Temporal.PlainYearMonth(2000, 5, calendar);
    const zonedDateTime = new Temporal.ZonedDateTime(1_000_000_000_000_000_000n, "UTC", calendar);

    [plainDate, plainDateTime, plainMonthDay, plainYearMonth, zonedDateTime].forEach((temporalObject) => {
      const actual = [];
      const expected = [];

      Object.defineProperty(temporalObject, "calendar", {
        get() {
          actual.push("get calendar");
          return calendar;
        },
      });

      func(temporalObject, calendar);
      assert.compareArray(actual, expected, "calendar getter not called");
    });
  },

  checkToTemporalInstantFastPath(func) {
    const actual = [];
    const expected = [];

    const datetime = new Temporal.ZonedDateTime(1_000_000_000_987_654_321n, "UTC");
    Object.defineProperty(datetime, 'toString', {
      get() {
        actual.push("get toString");
        return function (options) {
          actual.push("call toString");
          return Temporal.ZonedDateTime.prototype.toString.call(this, options);
        };
      },
    });

    func(datetime);
    assert.compareArray(actual, expected, "toString not called");
  },

  checkToTemporalPlainDateTimeFastPath(func) {
    const actual = [];
    const expected = [];

    const calendar = new Temporal.Calendar("iso8601");
    const date = new Temporal.PlainDate(2000, 5, 2, calendar);
    const prototypeDescrs = Object.getOwnPropertyDescriptors(Temporal.PlainDate.prototype);
    ["year", "month", "monthCode", "day"].forEach((property) => {
      Object.defineProperty(date, property, {
        get() {
          actual.push(`get ${formatPropertyName(property)}`);
          const value = prototypeDescrs[property].get.call(this);
          return TemporalHelpers.toPrimitiveObserver(actual, value, property);
        },
      });
    });
    ["hour", "minute", "second", "millisecond", "microsecond", "nanosecond"].forEach((property) => {
      Object.defineProperty(date, property, {
        get() {
          actual.push(`get ${formatPropertyName(property)}`);
          return undefined;
        },
      });
    });
    Object.defineProperty(date, "calendar", {
      get() {
        actual.push("get calendar");
        return calendar;
      },
    });

    func(date, calendar);
    assert.compareArray(actual, expected, "property getters not called");
  },

  /*
   * A custom calendar used in prototype pollution checks. Verifies that the
   * fromFields methods are always called with a null-prototype fields object.
   */
  calendarCheckFieldsPrototypePollution() {
    class CalendarCheckFieldsPrototypePollution extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.dateFromFieldsCallCount = 0;
        this.yearMonthFromFieldsCallCount = 0;
        this.monthDayFromFieldsCallCount = 0;
      }

      // toString must remain "iso8601", so that some methods don't throw due to
      // incompatible calendars

      dateFromFields(fields, options = {}) {
        this.dateFromFieldsCallCount++;
        assert.sameValue(Object.getPrototypeOf(fields), null, "dateFromFields should be called with null-prototype fields object");
        return super.dateFromFields(fields, options);
      }

      yearMonthFromFields(fields, options = {}) {
        this.yearMonthFromFieldsCallCount++;
        assert.sameValue(Object.getPrototypeOf(fields), null, "yearMonthFromFields should be called with null-prototype fields object");
        return super.yearMonthFromFields(fields, options);
      }

      monthDayFromFields(fields, options = {}) {
        this.monthDayFromFieldsCallCount++;
        assert.sameValue(Object.getPrototypeOf(fields), null, "monthDayFromFields should be called with null-prototype fields object");
        return super.monthDayFromFields(fields, options);
      }
    }

    return new CalendarCheckFieldsPrototypePollution();
  },

  /*
   * A custom calendar used in prototype pollution checks. Verifies that the
   * mergeFields() method is always called with null-prototype fields objects.
   */
  calendarCheckMergeFieldsPrototypePollution() {
    class CalendarCheckMergeFieldsPrototypePollution extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.mergeFieldsCallCount = 0;
      }

      toString() {
        return "merge-fields-null-proto";
      }

      mergeFields(fields, additionalFields) {
        this.mergeFieldsCallCount++;
        assert.sameValue(Object.getPrototypeOf(fields), null, "mergeFields should be called with null-prototype fields object (first argument)");
        assert.sameValue(Object.getPrototypeOf(additionalFields), null, "mergeFields should be called with null-prototype fields object (second argument)");
        return super.mergeFields(fields, additionalFields);
      }
    }

    return new CalendarCheckMergeFieldsPrototypePollution();
  },

  /*
   * A custom calendar used in prototype pollution checks. Verifies that methods
   * are always called with a null-prototype options object.
   */
  calendarCheckOptionsPrototypePollution() {
    class CalendarCheckOptionsPrototypePollution extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.yearMonthFromFieldsCallCount = 0;
        this.dateUntilCallCount = 0;
      }

      toString() {
        return "options-null-proto";
      }

      yearMonthFromFields(fields, options) {
        this.yearMonthFromFieldsCallCount++;
        assert.sameValue(Object.getPrototypeOf(options), null, "yearMonthFromFields should be called with null-prototype options");
        return super.yearMonthFromFields(fields, options);
      }

      dateUntil(one, two, options) {
        this.dateUntilCallCount++;
        assert.sameValue(Object.getPrototypeOf(options), null, "dateUntil should be called with null-prototype options");
        return super.dateUntil(one, two, options);
      }
    }

    return new CalendarCheckOptionsPrototypePollution();
  },

  /*
   * A custom calendar that asserts its dateAdd() method is called with the
   * options parameter having the value undefined.
   */
  calendarDateAddUndefinedOptions() {
    class CalendarDateAddUndefinedOptions extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.dateAddCallCount = 0;
      }

      toString() {
        return "dateadd-undef-options";
      }

      dateAdd(date, duration, options) {
        this.dateAddCallCount++;
        assert.sameValue(options, undefined, "dateAdd shouldn't be called with options");
        return super.dateAdd(date, duration, options);
      }
    }
    return new CalendarDateAddUndefinedOptions();
  },

  /*
   * A custom calendar that asserts its dateAdd() method is called with a
   * PlainDate instance. Optionally, it also asserts that the PlainDate instance
   * is the specific object `this.specificPlainDate`, if it is set by the
   * calling code.
   */
  calendarDateAddPlainDateInstance() {
    class CalendarDateAddPlainDateInstance extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.dateAddCallCount = 0;
        this.specificPlainDate = undefined;
      }

      toString() {
        return "dateadd-plain-date-instance";
      }

      dateFromFields(...args) {
        return super.dateFromFields(...args).withCalendar(this);
      }

      dateAdd(date, duration, options) {
        this.dateAddCallCount++;
        assert(date instanceof Temporal.PlainDate, "dateAdd() should be called with a PlainDate instance");
        if (this.dateAddCallCount === 1 && this.specificPlainDate) {
          assert.sameValue(date, this.specificPlainDate, `dateAdd() should be called first with the specific PlainDate instance ${this.specificPlainDate}`);
        }
        return super.dateAdd(date, duration, options).withCalendar(this);
      }
    }
    return new CalendarDateAddPlainDateInstance();
  },

  /*
   * A custom calendar that returns an iterable instead of an array from its
   * fields() method, otherwise identical to the ISO calendar.
   */
  calendarFieldsIterable() {
    class CalendarFieldsIterable extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.fieldsCallCount = 0;
        this.fieldsCalledWith = [];
        this.iteratorExhausted = [];
      }

      toString() {
        return "fields-iterable";
      }

      fields(fieldNames) {
        this.fieldsCallCount++;
        this.fieldsCalledWith.push(fieldNames.slice());
        this.iteratorExhausted.push(false);
        return {
          callIndex: this.fieldsCallCount - 1,
          calendar: this,
          *[Symbol.iterator]() {
            yield* this.calendar.fieldsCalledWith[this.callIndex];
            this.calendar.iteratorExhausted[this.callIndex] = true;
          },
        };
      }
    }
    return new CalendarFieldsIterable();
  },

  /*
   * A custom calendar that asserts its ...FromFields() methods are called with
   * the options parameter having the value undefined.
   */
  calendarFromFieldsUndefinedOptions() {
    class CalendarFromFieldsUndefinedOptions extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.dateFromFieldsCallCount = 0;
        this.monthDayFromFieldsCallCount = 0;
        this.yearMonthFromFieldsCallCount = 0;
      }

      toString() {
        return "from-fields-undef-options";
      }

      dateFromFields(fields, options) {
        this.dateFromFieldsCallCount++;
        assert.sameValue(options, undefined, "dateFromFields shouldn't be called with options");
        return super.dateFromFields(fields, options);
      }

      yearMonthFromFields(fields, options) {
        this.yearMonthFromFieldsCallCount++;
        assert.sameValue(options, undefined, "yearMonthFromFields shouldn't be called with options");
        return super.yearMonthFromFields(fields, options);
      }

      monthDayFromFields(fields, options) {
        this.monthDayFromFieldsCallCount++;
        assert.sameValue(options, undefined, "monthDayFromFields shouldn't be called with options");
        return super.monthDayFromFields(fields, options);
      }
    }
    return new CalendarFromFieldsUndefinedOptions();
  },

  /*
   * A custom calendar that modifies the fields object passed in to
   * dateFromFields, sabotaging its time properties.
   */
  calendarMakeInfinityTime() {
    class CalendarMakeInfinityTime extends Temporal.Calendar {
      constructor() {
        super("iso8601");
      }

      dateFromFields(fields, options) {
        const retval = super.dateFromFields(fields, options);
        fields.hour = Infinity;
        fields.minute = Infinity;
        fields.second = Infinity;
        fields.millisecond = Infinity;
        fields.microsecond = Infinity;
        fields.nanosecond = Infinity;
        return retval;
      }
    }
    return new CalendarMakeInfinityTime();
  },

  /*
   * A custom calendar that defines getters on the fields object passed into
   * dateFromFields that throw, sabotaging its time properties.
   */
  calendarMakeInvalidGettersTime() {
    class CalendarMakeInvalidGettersTime extends Temporal.Calendar {
      constructor() {
        super("iso8601");
      }

      dateFromFields(fields, options) {
        const retval = super.dateFromFields(fields, options);
        const throwingDescriptor = {
          get() {
            throw new Test262Error("reading a sabotaged time field");
          },
        };
        Object.defineProperties(fields, {
          hour: throwingDescriptor,
          minute: throwingDescriptor,
          second: throwingDescriptor,
          millisecond: throwingDescriptor,
          microsecond: throwingDescriptor,
          nanosecond: throwingDescriptor,
        });
        return retval;
      }
    }
    return new CalendarMakeInvalidGettersTime();
  },

  /*
   * A custom calendar whose mergeFields() method returns a proxy object with
   * all of its Get and HasProperty operations observable, as well as adding a
   * "shouldNotBeCopied": true property.
   */
  calendarMergeFieldsGetters() {
    class CalendarMergeFieldsGetters extends Temporal.Calendar {
      constructor() {
        super("iso8601");
        this.mergeFieldsReturnOperations = [];
      }

      toString() {
        return "merge-fields-getters";
      }

      dateFromFields(fields, options) {
        assert.sameValue(fields.shouldNotBeCopied, undefined, "extra fields should not be copied");
        return super.dateFromFields(fields, options);
      }

      yearMonthFromFields(fields, options) {
        assert.sameValue(fields.shouldNotBeCopied, undefined, "extra fields should not be copied");
        return super.yearMonthFromFields(fields, options);
      }

      monthDayFromFields(fields, options) {
        assert.sameValue(fields.shouldNotBeCopied, undefined, "extra fields should not be copied");
        return super.monthDayFromFields(fields, options);
      }

      mergeFields(fields, additionalFields) {
        const retval = super.mergeFields(fields, additionalFields);
        retval._calendar = this;
        retval.shouldNotBeCopied = true;
        return new Proxy(retval, {
          get(target, key) {
            target._calendar.mergeFieldsReturnOperations.push(`get ${key}`);
            const result = target[key];
            if (result === undefined) {
              return undefined;
            }
            return TemporalHelpers.toPrimitiveObserver(target._calendar.mergeFieldsReturnOperations, result, key);
          },
          has(target, key) {
            target._calendar.mergeFieldsReturnOperations.push(`has ${key}`);
            return key in target;
          },
        });
      }
    }
    return new CalendarMergeFieldsGetters();
  },

  /*
   * A custom calendar whose mergeFields() method returns a primitive value,
   * given by @primitive, and which records the number of calls made to its
   * dateFromFields(), yearMonthFromFields(), and monthDayFromFields() methods.
   */
  calendarMergeFieldsReturnsPrimitive(primitive) {
    class CalendarMergeFieldsPrimitive extends Temporal.Calendar {
      constructor(mergeFieldsReturnValue) {
        super("iso8601");
        this._mergeFieldsReturnValue = mergeFieldsReturnValue;
        this.dateFromFieldsCallCount = 0;
        this.monthDayFromFieldsCallCount = 0;
        this.yearMonthFromFieldsCallCount = 0;
      }

      toString() {
        return "merge-fields-primitive";
      }

      dateFromFields(fields, options) {
        this.dateFromFieldsCallCount++;
        return super.dateFromFields(fields, options);
      }

      yearMonthFromFields(fields, options) {
        this.yearMonthFromFieldsCallCount++;
        return super.yearMonthFromFields(fields, options);
      }

      monthDayFromFields(fields, options) {
        this.monthDayFromFieldsCallCount++;
        return super.monthDayFromFields(fields, options);
      }

      mergeFields() {
        return this._mergeFieldsReturnValue;
      }
    }
    return new CalendarMergeFieldsPrimitive(primitive);
  },

  /*
   * A custom calendar whose fields() method returns the same value as the
   * iso8601 calendar, with the addition of extraFields provided as parameter.
   */
  calendarWithExtraFields(fields) {
    class CalendarWithExtraFields extends Temporal.Calendar {
      constructor(extraFields) {
        super("iso8601");
        this._extraFields = extraFields;
      }

      fields(fieldNames) {
        return super.fields(fieldNames).concat(this._extraFields);
      }
    }

    return new CalendarWithExtraFields(fields);
  },

  /*
   * crossDateLineTimeZone():
   *
   * This returns an instance of a custom time zone class that implements one
   * single transition where the time zone moves from one side of the
   * International Date Line to the other, for the purpose of testing time zone
   * calculations without depending on system time zone data.
   *
   * The transition occurs at epoch second 1325239200 and goes from offset
   * -10:00 to +14:00. In other words, the time zone skips the whole calendar
   * day of 2011-12-30. This is the same as the real-life transition in the
   * Pacific/Apia time zone.
   */
  crossDateLineTimeZone() {
    const { compare } = Temporal.PlainDate;
    const skippedDay = new Temporal.PlainDate(2011, 12, 30);
    const transitionEpoch = 1325239200_000_000_000n;
    const beforeOffset = new Temporal.TimeZone("-10:00");
    const afterOffset = new Temporal.TimeZone("+14:00");

    class CrossDateLineTimeZone extends Temporal.TimeZone {
      constructor() {
        super("+14:00");
      }

      getOffsetNanosecondsFor(instant) {
        if (instant.epochNanoseconds < transitionEpoch) {
          return beforeOffset.getOffsetNanosecondsFor(instant);
        }
        return afterOffset.getOffsetNanosecondsFor(instant);
      }

      getPossibleInstantsFor(datetime) {
        const comparison = compare(datetime.toPlainDate(), skippedDay);
        if (comparison === 0) {
          return [];
        }
        if (comparison < 0) {
          return [beforeOffset.getInstantFor(datetime)];
        }
        return [afterOffset.getInstantFor(datetime)];
      }

      getPreviousTransition(instant) {
        if (instant.epochNanoseconds > transitionEpoch) return new Temporal.Instant(transitionEpoch);
        return null;
      }

      getNextTransition(instant) {
        if (instant.epochNanoseconds < transitionEpoch) return new Temporal.Instant(transitionEpoch);
        return null;
      }

      toString() {
        return "Custom/Date_Line";
      }
    }
    return new CrossDateLineTimeZone();
  },

  /*
   * observeProperty(calls, object, propertyName, value):
   *
   * Defines an own property @object.@propertyName with value @value, that
   * will log any calls to its accessors to the array @calls.
   */
  observeProperty(calls, object, propertyName, value, objectName = "") {
    Object.defineProperty(object, propertyName, {
      get() {
        calls.push(`get ${formatPropertyName(propertyName, objectName)}`);
        return value;
      },
      set(v) {
        calls.push(`set ${formatPropertyName(propertyName, objectName)}`);
      }
    });
  },

  /*
   * observeMethod(calls, object, propertyName, value):
   *
   * Defines an own property @object.@propertyName with value @value, that
   * will log any calls of @value to the array @calls.
   */
  observeMethod(calls, object, propertyName, objectName = "") {
    const method = object[propertyName];
    object[propertyName] = function () {
      calls.push(`call ${formatPropertyName(propertyName, objectName)}`);
      return method.apply(object, arguments);
    };
  },

  /*
   * Used for substituteMethod to indicate default behavior instead of a
   * substituted value
   */
  SUBSTITUTE_SKIP: SKIP_SYMBOL,

  /*
   * substituteMethod(object, propertyName, values):
   *
   * Defines an own property @object.@propertyName that will, for each
   * subsequent call to the method previously defined as
   * @object.@propertyName:
   *  - Call the method, if no more values remain
   *  - Call the method, if the value in @values for the corresponding call
   *    is SUBSTITUTE_SKIP
   *  - Otherwise, return the corresponding value in @value
   */
  substituteMethod(object, propertyName, values) {
    let calls = 0;
    const method = object[propertyName];
    object[propertyName] = function () {
      if (calls >= values.length) {
        return method.apply(object, arguments);
      } else if (values[calls] === SKIP_SYMBOL) {
        calls++;
        return method.apply(object, arguments);
      } else {
        return values[calls++];
      }
    };
  },

  /*
   * calendarObserver:
   * A custom calendar that behaves exactly like the ISO 8601 calendar but
   * tracks calls to any of its methods, and Get/Has operations on its
   * properties, by appending messages to an array. This is for the purpose of
   * testing order of operations that are observable from user code.
   * objectName is used in the log.
   */
  calendarObserver(calls, objectName, methodOverrides = {}) {
    function removeExtraHasPropertyChecks(objectName, calls) {
      // Inserting the tracking calendar into the return values of methods
      // that we chain up into the ISO calendar for, causes extra HasProperty
      // checks, which we observe. This removes them so that we don't leak
      // implementation details of the helper into the test code.
      assert.sameValue(calls.pop(), `has ${objectName}.yearOfWeek`);
      assert.sameValue(calls.pop(), `has ${objectName}.yearMonthFromFields`);
      assert.sameValue(calls.pop(), `has ${objectName}.year`);
      assert.sameValue(calls.pop(), `has ${objectName}.weekOfYear`);
      assert.sameValue(calls.pop(), `has ${objectName}.monthsInYear`);
      assert.sameValue(calls.pop(), `has ${objectName}.monthDayFromFields`);
      assert.sameValue(calls.pop(), `has ${objectName}.monthCode`);
      assert.sameValue(calls.pop(), `has ${objectName}.month`);
      assert.sameValue(calls.pop(), `has ${objectName}.mergeFields`);
      assert.sameValue(calls.pop(), `has ${objectName}.inLeapYear`);
      assert.sameValue(calls.pop(), `has ${objectName}.id`);
      assert.sameValue(calls.pop(), `has ${objectName}.fields`);
      assert.sameValue(calls.pop(), `has ${objectName}.daysInYear`);
      assert.sameValue(calls.pop(), `has ${objectName}.daysInWeek`);
      assert.sameValue(calls.pop(), `has ${objectName}.daysInMonth`);
      assert.sameValue(calls.pop(), `has ${objectName}.dayOfYear`);
      assert.sameValue(calls.pop(), `has ${objectName}.dayOfWeek`);
      assert.sameValue(calls.pop(), `has ${objectName}.day`);
      assert.sameValue(calls.pop(), `has ${objectName}.dateUntil`);
      assert.sameValue(calls.pop(), `has ${objectName}.dateFromFields`);
      assert.sameValue(calls.pop(), `has ${objectName}.dateAdd`);
    }

    const iso8601 = new Temporal.Calendar("iso8601");
    const trackingMethods = {
      dateFromFields(...args) {
        calls.push(`call ${objectName}.dateFromFields`);
        if ('dateFromFields' in methodOverrides) {
          const value = methodOverrides.dateFromFields;
          return typeof value === "function" ? value(...args) : value;
        }
        const originalResult = iso8601.dateFromFields(...args);
        // Replace the calendar in the result with the call-tracking calendar
        const {isoYear, isoMonth, isoDay} = originalResult.getISOFields();
        const result = new Temporal.PlainDate(isoYear, isoMonth, isoDay, this);
        removeExtraHasPropertyChecks(objectName, calls);
        return result;
      },
      yearMonthFromFields(...args) {
        calls.push(`call ${objectName}.yearMonthFromFields`);
        if ('yearMonthFromFields' in methodOverrides) {
          const value = methodOverrides.yearMonthFromFields;
          return typeof value === "function" ? value(...args) : value;
        }
        const originalResult = iso8601.yearMonthFromFields(...args);
        // Replace the calendar in the result with the call-tracking calendar
        const {isoYear, isoMonth, isoDay} = originalResult.getISOFields();
        const result = new Temporal.PlainYearMonth(isoYear, isoMonth, this, isoDay);
        removeExtraHasPropertyChecks(objectName, calls);
        return result;
      },
      monthDayFromFields(...args) {
        calls.push(`call ${objectName}.monthDayFromFields`);
        if ('monthDayFromFields' in methodOverrides) {
          const value = methodOverrides.monthDayFromFields;
          return typeof value === "function" ? value(...args) : value;
        }
        const originalResult = iso8601.monthDayFromFields(...args);
        // Replace the calendar in the result with the call-tracking calendar
        const {isoYear, isoMonth, isoDay} = originalResult.getISOFields();
        const result = new Temporal.PlainMonthDay(isoMonth, isoDay, this, isoYear);
        removeExtraHasPropertyChecks(objectName, calls);
        return result;
      },
      dateAdd(...args) {
        calls.push(`call ${objectName}.dateAdd`);
        if ('dateAdd' in methodOverrides) {
          const value = methodOverrides.dateAdd;
          return typeof value === "function" ? value(...args) : value;
        }
        const originalResult = iso8601.dateAdd(...args);
        const {isoYear, isoMonth, isoDay} = originalResult.getISOFields();
        const result = new Temporal.PlainDate(isoYear, isoMonth, isoDay, this);
        removeExtraHasPropertyChecks(objectName, calls);
        return result;
      },
      id: "iso8601",
    };
    // Automatically generate the other methods that don't need any custom code
    [
      "dateUntil",
      "day",
      "dayOfWeek",
      "dayOfYear",
      "daysInMonth",
      "daysInWeek",
      "daysInYear",
      "era",
      "eraYear",
      "fields",
      "inLeapYear",
      "mergeFields",
      "month",
      "monthCode",
      "monthsInYear",
      "toString",
      "weekOfYear",
      "year",
      "yearOfWeek",
    ].forEach((methodName) => {
      trackingMethods[methodName] = function (...args) {
        calls.push(`call ${formatPropertyName(methodName, objectName)}`);
        if (methodName in methodOverrides) {
          const value = methodOverrides[methodName];
          return typeof value === "function" ? value(...args) : value;
        }
        return iso8601[methodName](...args);
      };
    });
    return new Proxy(trackingMethods, {
      get(target, key, receiver) {
        const result = Reflect.get(target, key, receiver);
        calls.push(`get ${formatPropertyName(key, objectName)}`);
        return result;
      },
      has(target, key) {
        calls.push(`has ${formatPropertyName(key, objectName)}`);
        return Reflect.has(target, key);
      },
    });
  },

  /*
   * A custom calendar that does not allow any of its methods to be called, for
   * the purpose of asserting that a particular operation does not call into
   * user code.
   */
  calendarThrowEverything() {
    class CalendarThrowEverything extends Temporal.Calendar {
      constructor() {
        super("iso8601");
      }
      toString() {
        TemporalHelpers.assertUnreachable("toString should not be called");
      }
      dateFromFields() {
        TemporalHelpers.assertUnreachable("dateFromFields should not be called");
      }
      yearMonthFromFields() {
        TemporalHelpers.assertUnreachable("yearMonthFromFields should not be called");
      }
      monthDayFromFields() {
        TemporalHelpers.assertUnreachable("monthDayFromFields should not be called");
      }
      dateAdd() {
        TemporalHelpers.assertUnreachable("dateAdd should not be called");
      }
      dateUntil() {
        TemporalHelpers.assertUnreachable("dateUntil should not be called");
      }
      era() {
        TemporalHelpers.assertUnreachable("era should not be called");
      }
      eraYear() {
        TemporalHelpers.assertUnreachable("eraYear should not be called");
      }
      year() {
        TemporalHelpers.assertUnreachable("year should not be called");
      }
      month() {
        TemporalHelpers.assertUnreachable("month should not be called");
      }
      monthCode() {
        TemporalHelpers.assertUnreachable("monthCode should not be called");
      }
      day() {
        TemporalHelpers.assertUnreachable("day should not be called");
      }
      fields() {
        TemporalHelpers.assertUnreachable("fields should not be called");
      }
      mergeFields() {
        TemporalHelpers.assertUnreachable("mergeFields should not be called");
      }
    }

    return new CalendarThrowEverything();
  },

  /*
   * oneShiftTimeZone(shiftInstant, shiftNanoseconds):
   *
   * In the case of a spring-forward time zone offset transition (skipped time),
   * and disambiguation === 'earlier', BuiltinTimeZoneGetInstantFor subtracts a
   * negative number of nanoseconds from a PlainDateTime, which should balance
   * with the microseconds field.
   *
   * This returns an instance of a custom time zone class which skips a length
   * of time equal to shiftNanoseconds (a number), at the Temporal.Instant
   * shiftInstant. Before shiftInstant, it's identical to UTC, and after
   * shiftInstant it's a constant-offset time zone.
   *
   * It provides a getPossibleInstantsForCalledWith member which is an array
   * with the result of calling toString() on any PlainDateTimes passed to
   * getPossibleInstantsFor().
   */
  oneShiftTimeZone(shiftInstant, shiftNanoseconds) {
    class OneShiftTimeZone extends Temporal.TimeZone {
      constructor(shiftInstant, shiftNanoseconds) {
        super("+00:00");
        this._shiftInstant = shiftInstant;
        this._epoch1 = shiftInstant.epochNanoseconds;
        this._epoch2 = this._epoch1 + BigInt(shiftNanoseconds);
        this._shiftNanoseconds = shiftNanoseconds;
        this._shift = new Temporal.Duration(0, 0, 0, 0, 0, 0, 0, 0, 0, this._shiftNanoseconds);
        this.getPossibleInstantsForCalledWith = [];
      }

      _isBeforeShift(instant) {
        return instant.epochNanoseconds < this._epoch1;
      }

      getOffsetNanosecondsFor(instant) {
        return this._isBeforeShift(instant) ? 0 : this._shiftNanoseconds;
      }

      getPossibleInstantsFor(plainDateTime) {
        this.getPossibleInstantsForCalledWith.push(plainDateTime.toString({ calendarName: "never" }));
        const [instant] = super.getPossibleInstantsFor(plainDateTime);
        if (this._shiftNanoseconds > 0) {
          if (this._isBeforeShift(instant)) return [instant];
          if (instant.epochNanoseconds < this._epoch2) return [];
          return [instant.subtract(this._shift)];
        }
        if (instant.epochNanoseconds < this._epoch2) return [instant];
        const shifted = instant.subtract(this._shift);
        if (this._isBeforeShift(instant)) return [instant, shifted];
        return [shifted];
      }

      getNextTransition(instant) {
        return this._isBeforeShift(instant) ? this._shiftInstant : null;
      }

      getPreviousTransition(instant) {
        return this._isBeforeShift(instant) ? null : this._shiftInstant;
      }

      toString() {
        return "Custom/One_Shift";
      }
    }
    return new OneShiftTimeZone(shiftInstant, shiftNanoseconds);
  },

  /*
   * propertyBagObserver():
   * Returns an object that behaves like the given propertyBag but tracks Get
   * and Has operations on any of its properties, by appending messages to an
   * array. If the value of a property in propertyBag is a primitive, the value
   * of the returned object's property will additionally be a
   * TemporalHelpers.toPrimitiveObserver that will track calls to its toString
   * and valueOf methods in the same array. This is for the purpose of testing
   * order of operations that are observable from user code. objectName is used
   * in the log.
   */
  propertyBagObserver(calls, propertyBag, objectName) {
    return new Proxy(propertyBag, {
      ownKeys(target) {
        calls.push(`ownKeys ${objectName}`);
        return Reflect.ownKeys(target);
      },
      getOwnPropertyDescriptor(target, key) {
        calls.push(`getOwnPropertyDescriptor ${formatPropertyName(key, objectName)}`);
        return Reflect.getOwnPropertyDescriptor(target, key);
      },
      get(target, key, receiver) {
        calls.push(`get ${formatPropertyName(key, objectName)}`);
        const result = Reflect.get(target, key, receiver);
        if (result === undefined) {
          return undefined;
        }
        if ((result !== null && typeof result === "object") || typeof result === "function") {
          return result;
        }
        return TemporalHelpers.toPrimitiveObserver(calls, result, `${formatPropertyName(key, objectName)}`);
      },
      has(target, key) {
        calls.push(`has ${formatPropertyName(key, objectName)}`);
        return Reflect.has(target, key);
      },
    });
  },

  /*
   * specificOffsetTimeZone():
   *
   * This returns an instance of a custom time zone class, which returns a
   * specific custom value from its getOffsetNanosecondsFrom() method. This is
   * for the purpose of testing the validation of what this method returns.
   *
   * It also returns an empty array from getPossibleInstantsFor(), so as to
   * trigger calls to getOffsetNanosecondsFor() when used from the
   * BuiltinTimeZoneGetInstantFor operation.
   */
  specificOffsetTimeZone(offsetValue) {
    class SpecificOffsetTimeZone extends Temporal.TimeZone {
      constructor(offsetValue) {
        super("UTC");
        this._offsetValue = offsetValue;
      }

      getOffsetNanosecondsFor() {
        return this._offsetValue;
      }

      getPossibleInstantsFor(dt) {
        if (typeof this._offsetValue !== 'number' || Math.abs(this._offsetValue) >= 86400e9 || isNaN(this._offsetValue)) return [];
        const zdt = dt.toZonedDateTime("UTC").add({ nanoseconds: -this._offsetValue });
        return [zdt.toInstant()];
      }

      get id() {
        return this.getOffsetStringFor(new Temporal.Instant(0n));
      }
    }
    return new SpecificOffsetTimeZone(offsetValue);
  },

  /*
   * springForwardFallBackTimeZone():
   *
   * This returns an instance of a custom time zone class that implements one
   * single spring-forward/fall-back transition, for the purpose of testing the
   * disambiguation option, without depending on system time zone data.
   *
   * The spring-forward occurs at epoch second 954669600 (2000-04-02T02:00
   * local) and goes from offset -08:00 to -07:00.
   *
   * The fall-back occurs at epoch second 972810000 (2000-10-29T02:00 local) and
   * goes from offset -07:00 to -08:00.
   */
  springForwardFallBackTimeZone() {
    const { compare } = Temporal.PlainDateTime;
    const springForwardLocal = new Temporal.PlainDateTime(2000, 4, 2, 2);
    const springForwardEpoch = 954669600_000_000_000n;
    const fallBackLocal = new Temporal.PlainDateTime(2000, 10, 29, 1);
    const fallBackEpoch = 972810000_000_000_000n;
    const winterOffset = new Temporal.TimeZone('-08:00');
    const summerOffset = new Temporal.TimeZone('-07:00');

    class SpringForwardFallBackTimeZone extends Temporal.TimeZone {
      constructor() {
        super("-08:00");
      }

      getOffsetNanosecondsFor(instant) {
        if (instant.epochNanoseconds < springForwardEpoch ||
          instant.epochNanoseconds >= fallBackEpoch) {
          return winterOffset.getOffsetNanosecondsFor(instant);
        }
        return summerOffset.getOffsetNanosecondsFor(instant);
      }

      getPossibleInstantsFor(datetime) {
        if (compare(datetime, springForwardLocal) >= 0 && compare(datetime, springForwardLocal.add({ hours: 1 })) < 0) {
          return [];
        }
        if (compare(datetime, fallBackLocal) >= 0 && compare(datetime, fallBackLocal.add({ hours: 1 })) < 0) {
          return [summerOffset.getInstantFor(datetime), winterOffset.getInstantFor(datetime)];
        }
        if (compare(datetime, springForwardLocal) < 0 || compare(datetime, fallBackLocal) >= 0) {
          return [winterOffset.getInstantFor(datetime)];
        }
        return [summerOffset.getInstantFor(datetime)];
      }

      getPreviousTransition(instant) {
        if (instant.epochNanoseconds > fallBackEpoch) return new Temporal.Instant(fallBackEpoch);
        if (instant.epochNanoseconds > springForwardEpoch) return new Temporal.Instant(springForwardEpoch);
        return null;
      }

      getNextTransition(instant) {
        if (instant.epochNanoseconds < springForwardEpoch) return new Temporal.Instant(springForwardEpoch);
        if (instant.epochNanoseconds < fallBackEpoch) return new Temporal.Instant(fallBackEpoch);
        return null;
      }

      get id() {
        return "Custom/Spring_Fall";
      }

      toString() {
        return "Custom/Spring_Fall";
      }
    }
    return new SpringForwardFallBackTimeZone();
  },

  /*
   * timeZoneObserver:
   * A custom calendar that behaves exactly like the UTC time zone but tracks
   * calls to any of its methods, and Get/Has operations on its properties, by
   * appending messages to an array. This is for the purpose of testing order of
   * operations that are observable from user code. objectName is used in the
   * log. methodOverrides is an optional object containing properties with the
   * same name as Temporal.TimeZone methods. If the property value is a function
   * it will be called with the proper arguments instead of the UTC method.
   * Otherwise, the property value will be returned directly.
   */
  timeZoneObserver(calls, objectName, methodOverrides = {}) {
    const utc = new Temporal.TimeZone("UTC");
    const trackingMethods = {
      id: "UTC",
    };
    // Automatically generate the methods
    ["getOffsetNanosecondsFor", "getPossibleInstantsFor", "toString"].forEach((methodName) => {
      trackingMethods[methodName] = function (...args) {
        calls.push(`call ${formatPropertyName(methodName, objectName)}`);
        if (methodName in methodOverrides) {
          const value = methodOverrides[methodName];
          return typeof value === "function" ? value(...args) : value;
        }
        return utc[methodName](...args);
      };
    });
    return new Proxy(trackingMethods, {
      get(target, key, receiver) {
        const result = Reflect.get(target, key, receiver);
        calls.push(`get ${formatPropertyName(key, objectName)}`);
        return result;
      },
      has(target, key) {
        calls.push(`has ${formatPropertyName(key, objectName)}`);
        return Reflect.has(target, key);
      },
    });
  },

  /*
   * A custom time zone that does not allow any of its methods to be called, for
   * the purpose of asserting that a particular operation does not call into
   * user code.
   */
  timeZoneThrowEverything() {
    class TimeZoneThrowEverything extends Temporal.TimeZone {
      constructor() {
        super("UTC");
      }
      getOffsetNanosecondsFor() {
        TemporalHelpers.assertUnreachable("getOffsetNanosecondsFor should not be called");
      }
      getPossibleInstantsFor() {
        TemporalHelpers.assertUnreachable("getPossibleInstantsFor should not be called");
      }
      toString() {
        TemporalHelpers.assertUnreachable("toString should not be called");
      }
    }

    return new TimeZoneThrowEverything();
  },

  /*
   * Returns an object that will append logs of any Gets or Calls of its valueOf
   * or toString properties to the array calls. Both valueOf and toString will
   * return the actual primitiveValue. propertyName is used in the log.
   */
  toPrimitiveObserver(calls, primitiveValue, propertyName) {
    return {
      get valueOf() {
        calls.push(`get ${propertyName}.valueOf`);
        return function () {
          calls.push(`call ${propertyName}.valueOf`);
          return primitiveValue;
        };
      },
      get toString() {
        calls.push(`get ${propertyName}.toString`);
        return function () {
          calls.push(`call ${propertyName}.toString`);
          if (primitiveValue === undefined) return undefined;
          return primitiveValue.toString();
        };
      },
    };
  },

  /*
   * An object containing further methods that return arrays of ISO strings, for
   * testing parsers.
   */
  ISO: {
    /*
     * PlainMonthDay strings that are not valid.
     */
    plainMonthDayStringsInvalid() {
      return [
        "11-18junk",
        "11-18[u-ca=gregory]",
        "11-18[u-ca=hebrew]",
      ];
    },

    /*
     * PlainMonthDay strings that are valid and that should produce October 1st.
     */
    plainMonthDayStringsValid() {
      return [
        "10-01",
        "1001",
        "1965-10-01",
        "1976-10-01T152330.1+00:00",
        "19761001T15:23:30.1+00:00",
        "1976-10-01T15:23:30.1+0000",
        "1976-10-01T152330.1+0000",
        "19761001T15:23:30.1+0000",
        "19761001T152330.1+00:00",
        "19761001T152330.1+0000",
        "+001976-10-01T152330.1+00:00",
        "+0019761001T15:23:30.1+00:00",
        "+001976-10-01T15:23:30.1+0000",
        "+001976-10-01T152330.1+0000",
        "+0019761001T15:23:30.1+0000",
        "+0019761001T152330.1+00:00",
        "+0019761001T152330.1+0000",
        "1976-10-01T15:23:00",
        "1976-10-01T15:23",
        "1976-10-01T15",
        "1976-10-01",
        "--10-01",
        "--1001",
      ];
    },

    /*
     * PlainTime strings that may be mistaken for PlainMonthDay or
     * PlainYearMonth strings, and so require a time designator.
     */
    plainTimeStringsAmbiguous() {
      const ambiguousStrings = [
        "2021-12",  // ambiguity between YYYY-MM and HHMM-UU
        "2021-12[-12:00]",  // ditto, TZ does not disambiguate
        "1214",     // ambiguity between MMDD and HHMM
        "0229",     //   ditto, including MMDD that doesn't occur every year
        "1130",     //   ditto, including DD that doesn't occur in every month
        "12-14",    // ambiguity between MM-DD and HH-UU
        "12-14[-14:00]",  // ditto, TZ does not disambiguate
        "202112",   // ambiguity between YYYYMM and HHMMSS
        "202112[UTC]",  // ditto, TZ does not disambiguate
      ];
      // Adding a calendar annotation to one of these strings must not cause
      // disambiguation in favour of time.
      const stringsWithCalendar = ambiguousStrings.map((s) => s + '[u-ca=iso8601]');
      return ambiguousStrings.concat(stringsWithCalendar);
    },

    /*
     * PlainTime strings that are of similar form to PlainMonthDay and
     * PlainYearMonth strings, but are not ambiguous due to components that
     * aren't valid as months or days.
     */
    plainTimeStringsUnambiguous() {
      return [
        "2021-13",          // 13 is not a month
        "202113",           //   ditto
        "2021-13[-13:00]",  //   ditto
        "202113[-13:00]",   //   ditto
        "0000-00",          // 0 is not a month
        "000000",           //   ditto
        "0000-00[UTC]",     //   ditto
        "000000[UTC]",      //   ditto
        "1314",             // 13 is not a month
        "13-14",            //   ditto
        "1232",             // 32 is not a day
        "0230",             // 30 is not a day in February
        "0631",             // 31 is not a day in June
        "0000",             // 0 is neither a month nor a day
        "00-00",            //   ditto
      ];
    },

    /*
     * PlainYearMonth-like strings that are not valid.
     */
    plainYearMonthStringsInvalid() {
      return [
        "2020-13",
      ];
    },

    /*
     * PlainYearMonth-like strings that are valid and should produce November
     * 1976 in the ISO 8601 calendar.
     */
    plainYearMonthStringsValid() {
      return [
        "1976-11",
        "1976-11-10",
        "1976-11-01T09:00:00+00:00",
        "1976-11-01T00:00:00+05:00",
        "197611",
        "+00197611",
        "1976-11-18T15:23:30.1\u221202:00",
        "1976-11-18T152330.1+00:00",
        "19761118T15:23:30.1+00:00",
        "1976-11-18T15:23:30.1+0000",
        "1976-11-18T152330.1+0000",
        "19761118T15:23:30.1+0000",
        "19761118T152330.1+00:00",
        "19761118T152330.1+0000",
        "+001976-11-18T152330.1+00:00",
        "+0019761118T15:23:30.1+00:00",
        "+001976-11-18T15:23:30.1+0000",
        "+001976-11-18T152330.1+0000",
        "+0019761118T15:23:30.1+0000",
        "+0019761118T152330.1+00:00",
        "+0019761118T152330.1+0000",
        "1976-11-18T15:23",
        "1976-11-18T15",
        "1976-11-18",
      ];
    },

    /*
     * PlainYearMonth-like strings that are valid and should produce November of
     * the ISO year -9999.
     */
    plainYearMonthStringsValidNegativeYear() {
      return [
        "\u2212009999-11",
      ];
    },
  }
};
