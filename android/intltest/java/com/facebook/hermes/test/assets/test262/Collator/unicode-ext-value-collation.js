// Copyright 2012 Mozilla Corporation. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
es5id: 10.1.1_19_b
description: Tests the special handling of the "co" key in Intl.Collator.
author: Norbert Lindenberg
---*/

function checkCollation(extensionCoValue, usageValue, expectedCollations, expectedUsage) {
    var requestLocale = extensionCoValue !== undefined ? "de-DE-u-co-" + extensionCoValue : "de-DE";
    var options = usageValue !== undefined ? { usage: usageValue } : undefined;
    var collator = new Intl.Collator([requestLocale], options);

    var collation = collator.resolvedOptions().collation;
    assert.notSameValue(expectedCollations.indexOf(collation), -1, (extensionCoValue === undefined ? "Default collation" : "Collation for \"" + extensionCoValue) + "\" should be " + expectedCollations.join(" or ") + ", but is " + collation + ".");

    var usage = collator.resolvedOptions().usage;
    assert.sameValue(usage, expectedUsage, (usageValue === undefined ? "Default usage" : "Usage") + " mismatch.");
}

checkCollation(undefined, undefined, ["default"], "sort");

checkCollation("phonebk", undefined, ["phonebk", "default"], "sort");

checkCollation("invalid", undefined, ["default"], "sort");

checkCollation("standard", undefined, ["default"], "sort");

checkCollation("standard", "search", ["default"], "search");

checkCollation("standard", "sort", ["default"], "sort");

checkCollation("search", undefined, ["default"], "sort");

checkCollation("search", "search", ["default"], "search");

checkCollation("search", "sort", ["default"], "sort");
