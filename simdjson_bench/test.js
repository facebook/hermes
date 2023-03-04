print("hello");
const hermesBuildType = HermesInternal.getRuntimeProperties().Build;
print(hermesBuildType);
const fs = require("fs");

// sanityChecks();
runTests();
// stressTest();

// -- implementation --

function getRec() {
  // has short strings, user text strings, ints, floats, nulls, and booleans
  return '{"id":"TveON9Jy24VvxURE","_status":0,"_changed":"last_seen_activity_at","author_id":"XhGTYQH9vjn1M8EU","created_at":1671397237868,"comments_cached":null,"due_at":null,"ended_at":null,"is_abandoned":false,"is_all_day":true,"is_followed":false,"last_activity_at":1673654298383,"last_reviewed_at":1674021667146,"last_seen_activity_at":1673654298383,"missed_repeats":0,"name":"Reason fail notice wonder stuff including although poor plant","priority_position":36,"project_id":"xOkNP17zoVVZRtFx","project_position":19.32784,"project_section_id":"N8PrUwpdVMYmRE2F","recurrence_id":null,"responsible_id":"lfSW3tfNhaeX71ln","review_reason":"delegated","review_triggered_at":1674879868553,"time_needed":0,"time_spent":0}';
}
function getRecUnicode() {
  return getRec().replace(
    "Reason fail notice wonder stuff including although poor plant",
    "Ręąśóń fąïl nötïcë wǒnděr śtüff ïńćlüdïñg ąłthöügh pơơr płąñt"
  );
}

function createNested(record) {
  return (
    '{"changes": { "tasks": { "created": [], "updated": [' +
    record +
    '], "deleted": ["a7hbehTKyKOQgfIn","hRQVAdbAXhbckXXh","p6HUHuu7ZV4MA0Ho"] } }, "timestamp": 1676630449891 }'
  );
}

function createLarge(record, count) {
  print("Preparing a very large JSON string...");
  const json =
    "[" +
    Array(count || (hermesBuildType === "Release" ? 25000 : 5000))
      .fill(record)
      .join(",") +
    "]";

  print(`Prepared. JSON size: ${Math.round(json.length / 1000 / 1000)}MB`);
  return json;
}

function runTests() {
  const formatTime = (time) =>
    time < 1 ? `${roundTo(time * 1000, 4)}µs` : `${roundTo(time, 4)}ms`;
  function testJSON(
    name,
    jsonString,
    _iterationCount = 10000,
    skipCheck = false
  ) {
    print(`=== Testing "${name}"...`);
    const iterationCount =
      _iterationCount * (hermesBuildType === "Release" ? 10 : 1);

    let b4, time;
    let lastParseResult, lastFastParseResult;

    b4 = Date.now();
    for (let i = 0; i < iterationCount; i++) {
      lastParseResult = JSON.parse(jsonString);
    }
    const time = (Date.now() - b4) / iterationCount;
    print(`JSON.parse: ${formatTime(time)}`);

    b4 = Date.now();
    for (let i = 0; i < iterationCount; i++) {
      lastFastParseResult = JSON.fastParse(jsonString);
    }
    const fastTime = (Date.now() - b4) / iterationCount;
    print(`JSON.fastParse: ${formatTime(fastTime)}`);
    print(`Speedup: ${roundTo(time / fastTime, 2)}x`);

    const equals =
      JSON.stringify(lastParseResult) === JSON.stringify(lastFastParseResult);
    if (!equals && !skipCheck) {
      throw new Error("JSON.fastParse is broken");
    }

    print("");
  }

  // NOTE: Too small to realiably measure
  testJSON("Integer", "0");
  testJSON("Float", "3.14152137");
  testJSON("Boolean", "true");
  testJSON("Null", "null");
  testJSON("String", '"hello world"');
  testJSON(
    "String (Unicode)",
    '"Ręąśóń fąïl nötïcë wǒnděr śtüff ïńćlüdïñg ąłthöügh pơơr płąñt"'
  );
  testJSON("Empty array", "[]");
  testJSON("Empty object", "{}");

  testJSON("Simple array", '[0, 1, true, false, null, "hello"]');
  testJSON(
    "Simple object",
    '{"int":0,"float":3.14152137,"bool":true,"string":"hello world"}'
  );

  testJSON(
    "Nested structure (ASCII)",
    JSON.debugToAscii(createNested(getRec())),
    1000
  );
  testJSON(
    "Nested structure (with Unicode)",
    createNested(getRecUnicode()),
    1000
  );

  const twitterJson = fs.readFileSync("twitter.json").toString("utf8");
  print(`twitter.json size: ${Math.round(twitterJson.length / 1000 / 1000)}MB`);
  testJSON("twitter.json", twitterJson, 20);

  const smallWatermelonDbJson = readFileMaybe("../../watermelondb-small-pretty-printed.json")
  if (smallWatermelonDbJson) {
    testJSON("Pretty-printed WatermelonDB Sync dump", smallWatermelonDbJson, 40);
  } else {
    print("Warning: missing watermelondb-small-pretty-printed.json")
  }

  const watermelonDbJson = readFileMaybe("../../watermelondb-nozbe-huge.json")
  if (watermelonDbJson) {
    print(
      `watermelondb-nozbe-huge.json size: ${Math.round(
        watermelonDbJson.length / 1000 / 1000
      )}MB`
    );
    testJSON("Large Nozbe account dump", watermelonDbJson, 1);
  } else {
    print("Warning: missing watermelondb-nozbe-huge.json")
  }

  testJSON(
    "Simulated large JSON (ASCII)",
    JSON.debugToAscii(createLarge(getRec())),
    2
  );
  testJSON(
    "Simulated large JSON (with Unicode)",
    createLarge(getRecUnicode()),
    2
  );
}

function readFileMaybe(path) {
  try {
    return fs
      .readFileSync(path)
      .toString("utf8");
  } catch (_e) {
    return null
  }
}

function sanityChecks() {
  print(JSON.fastParse("-1"));
  print(JSON.fastParse("3.14"));
  print(JSON.fastParse("true"));
  print(JSON.fastParse("false"));
  print(JSON.fastParse("null"));
  print(JSON.fastParse('"hello from json"'));
  print(JSON.stringify(JSON.fastParse("[]")));
  print(JSON.stringify(JSON.fastParse("[1,2,3]")));
  print(
    JSON.stringify(JSON.fastParse('["hello",true,false,null,3.14,[1,2,3]]'))
  );
  print(JSON.stringify(JSON.fastParse('{"foo":true,"bar":"hello"}')));
  print(
    JSON.stringify(
      JSON.fastParse(
        '{"int":0,"float":3.14152137,"bool":true,"string":"hello world"}'
      )
    )
  );

  print(JSON.fastParse('"ąść"'));
  print(
    JSON.stringify(JSON.fastParse('{"a":"hello","b":"world","a":"overriden"}'))
  );
  print(JSON.stringify(JSON.fastParse('{"ąść":true}')));

  for (let i = 0; i < 100000; i++) {
    JSON.fastParse('{"aa":"aa"}');
  }
  print("check1.");

  for (let i = 0; i < 500; i++) {
    JSON.fastParse(
      '[{"id":"TveON9Jy24VvxURE","_status":0,"_changed":"last_seen_activity_at","author_id":"XhGTYQH9vjn1M8EU","created_at":1671397237868,"comments_cached":null,"due_at":null,"ended_at":null,"is_abandoned":false,"is_all_day":false,"is_followed":false,"last_activity_at":1673654298383,"last_reviewed_at":1674021667146,"last_seen_activity_at":1673654298383,"missed_repeats":0,"name":"Reason fail notice wonder stuff including although poor plant","priority_position":36,"project_id":"xOkNP17zoVVZRtFx","project_position":19.32784,"project_section_id":"N8PrUwpdVMYmRE2F","recurrence_id":null,"responsible_id":"lfSW3tfNhaeX71ln","review_reason":"delegated","review_triggered_at":1674879868553,"time_needed":0,"time_spent":0}]'
    );
  }
  print("check2.");

  print("done.");
}

function roundTo(num, digits) {
  const x = Math.pow(10, digits);
  return Math.round(num * x) / x;
}

function stressTest() {
  const json = JSON.debugToAscii(createLarge(getRec()));
  let lastParseResult;
  b4 = Date.now();
  for (let i = 0; i < 40; i++) {
    lastParseResult = JSON.fastParse(json);
  }
  print(`done in ${Date.now() - b4}ms`);
}
