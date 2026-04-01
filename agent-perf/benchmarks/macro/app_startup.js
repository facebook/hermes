/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Macro-benchmark: Comprehensive app startup simulation.
// Combines configuration initialization, event system setup, route table
// building, component registration, and data store initialization.
// Exercises object creation, closures, map lookups, array operations,
// string processing, and the interplay of multiple subsystems.

"use strict";

var ITERATIONS = 300;

// ============================================================
// Subsystem 1: Configuration
// ============================================================

function createConfig() {
  var config = {
    app: {
      name: "BenchmarkApp",
      version: "3.2.1",
      buildNumber: 42,
      debug: false
    },
    api: {
      baseUrl: "https://api.example.com",
      timeout: 30000,
      retries: 3,
      headers: {
        "Content-Type": "application/json",
        "Accept": "application/json",
        "X-App-Version": "3.2.1"
      }
    },
    features: {
      darkMode: true,
      notifications: true,
      analytics: true,
      crashReporting: true,
      offlineMode: false
    },
    cache: {
      maxAge: 3600000,
      maxEntries: 500,
      strategies: ["memory", "disk"]
    },
    i18n: {
      defaultLocale: "en-US",
      supportedLocales: ["en-US", "es-ES", "fr-FR", "de-DE", "ja-JP",
                         "ko-KR", "zh-CN", "pt-BR", "it-IT", "ru-RU"]
    }
  };

  // Merge environment overrides
  var overrides = {debug: true, api: {timeout: 5000}};
  config.app.debug = overrides.debug;
  config.api.timeout = overrides.api.timeout;

  return config;
}

// ============================================================
// Subsystem 2: Event System
// ============================================================

function createEventSystem() {
  var listeners = {};
  var onceListeners = {};

  function on(event, callback, priority) {
    if (!listeners[event]) {
      listeners[event] = [];
    }
    listeners[event].push({
      callback: callback,
      priority: priority || 0
    });
    // Keep sorted by priority (descending)
    listeners[event].sort(function(a, b) {
      return b.priority - a.priority;
    });
  }

  function once(event, callback) {
    if (!onceListeners[event]) {
      onceListeners[event] = [];
    }
    onceListeners[event].push(callback);
  }

  function emit(event, data) {
    var results = [];
    var list = listeners[event];
    if (list) {
      for (var i = 0; i < list.length; i++) {
        results.push(list[i].callback(data));
      }
    }
    var onceList = onceListeners[event];
    if (onceList) {
      for (var j = 0; j < onceList.length; j++) {
        results.push(onceList[j](data));
      }
      onceListeners[event] = [];
    }
    return results;
  }

  function listenerCount(event) {
    var count = 0;
    if (listeners[event]) count += listeners[event].length;
    if (onceListeners[event]) count += onceListeners[event].length;
    return count;
  }

  return {on: on, once: once, emit: emit, listenerCount: listenerCount};
}

function registerEventListeners(events) {
  var eventNames = [
    "app:init", "app:ready", "app:pause", "app:resume", "app:error",
    "nav:push", "nav:pop", "nav:replace", "nav:reset",
    "data:fetch", "data:update", "data:delete", "data:sync",
    "ui:render", "ui:layout", "ui:scroll", "ui:touch",
    "net:online", "net:offline", "net:request", "net:response"
  ];

  for (var i = 0; i < eventNames.length; i++) {
    var name = eventNames[i];
    // Register 3 listeners per event at different priorities
    for (var p = 0; p < 3; p++) {
      (function(evtName, priority) {
        events.on(evtName, function(data) {
          return {event: evtName, priority: priority, handled: true};
        }, priority * 5);
      })(name, p);
    }
  }

  // Register some one-time listeners
  events.once("app:init", function() { return {initialized: true}; });
  events.once("app:ready", function() { return {ready: true}; });

  return eventNames.length;
}

// ============================================================
// Subsystem 3: Route Table
// ============================================================

function createRouter() {
  var routes = [];
  var routeMap = {};

  function addRoute(path, handler, middleware) {
    var segments = path.split("/").filter(function(s) { return s.length > 0; });
    var paramNames = [];
    for (var i = 0; i < segments.length; i++) {
      if (segments[i][0] === ":") {
        paramNames.push(segments[i].substring(1));
      }
    }
    var route = {
      path: path,
      segments: segments,
      paramNames: paramNames,
      handler: handler,
      middleware: middleware || []
    };
    routes.push(route);
    routeMap[path] = route;
  }

  function match(url) {
    var urlSegments = url.split("/").filter(function(s) { return s.length > 0; });
    for (var i = 0; i < routes.length; i++) {
      var route = routes[i];
      if (route.segments.length !== urlSegments.length) continue;

      var params = {};
      var matched = true;
      for (var j = 0; j < route.segments.length; j++) {
        if (route.segments[j][0] === ":") {
          params[route.segments[j].substring(1)] = urlSegments[j];
        } else if (route.segments[j] !== urlSegments[j]) {
          matched = false;
          break;
        }
      }

      if (matched) {
        return {route: route, params: params};
      }
    }
    return null;
  }

  return {addRoute: addRoute, match: match, routes: routes};
}

function registerRoutes(router) {
  var sections = ["home", "profile", "settings", "search", "messages",
                  "notifications", "feed", "explore", "create", "admin"];

  for (var i = 0; i < sections.length; i++) {
    var section = sections[i];
    var authMiddleware = function(ctx) { ctx.authenticated = true; };
    var logMiddleware = function(ctx) { ctx.logged = true; };

    // List route
    router.addRoute("/" + section, function(params) {
      return {view: section + "List"};
    }, [logMiddleware]);

    // Detail route
    router.addRoute("/" + section + "/:id", function(params) {
      return {view: section + "Detail", id: params.id};
    }, [authMiddleware, logMiddleware]);

    // Edit route
    router.addRoute("/" + section + "/:id/edit", function(params) {
      return {view: section + "Edit", id: params.id};
    }, [authMiddleware]);

    // Nested route
    router.addRoute("/" + section + "/:id/comments", function(params) {
      return {view: section + "Comments", id: params.id};
    }, [logMiddleware]);
  }

  return sections.length * 4;
}

// ============================================================
// Subsystem 4: Component Registry
// ============================================================

function createComponentRegistry() {
  var components = {};
  var instances = [];

  function register(name, factory) {
    components[name] = {
      name: name,
      factory: factory,
      instanceCount: 0
    };
  }

  function create(name, props) {
    var comp = components[name];
    if (!comp) return null;
    comp.instanceCount++;
    var instance = comp.factory(props);
    instance._name = name;
    instance._id = instances.length;
    instances.push(instance);
    return instance;
  }

  function getStats() {
    var names = Object.keys(components);
    var stats = {};
    for (var i = 0; i < names.length; i++) {
      stats[names[i]] = components[names[i]].instanceCount;
    }
    return stats;
  }

  return {register: register, create: create, getStats: getStats};
}

function registerComponents(registry) {
  var componentNames = [
    "Button", "Text", "View", "Image", "ScrollView",
    "TextInput", "FlatList", "Modal", "TouchableOpacity", "Switch",
    "ActivityIndicator", "Alert", "Badge", "Card", "Checkbox",
    "Divider", "Dropdown", "Header", "Icon", "Label",
    "Link", "Menu", "Picker", "ProgressBar", "Radio",
    "Slider", "Spinner", "Tab", "Toast", "Tooltip"
  ];

  for (var i = 0; i < componentNames.length; i++) {
    (function(name) {
      registry.register(name, function(props) {
        return {
          type: name,
          props: Object.assign({}, props),
          state: {mounted: false, visible: true},
          render: function() {
            return {type: name, props: this.props, children: []};
          },
          mount: function() { this.state.mounted = true; },
          unmount: function() { this.state.mounted = false; }
        };
      });
    })(componentNames[i]);
  }

  return componentNames.length;
}

// ============================================================
// Subsystem 5: Data Store
// ============================================================

function createDataStore() {
  var collections = {};
  var indices = {};
  var subscribers = [];

  function createCollection(name) {
    collections[name] = {};
    indices[name] = {};
  }

  function insert(collection, item) {
    if (!collections[collection]) createCollection(collection);
    collections[collection][item.id] = item;
    // Update indices
    var keys = Object.keys(item);
    for (var i = 0; i < keys.length; i++) {
      var key = keys[i];
      if (key === "id") continue;
      if (!indices[collection][key]) {
        indices[collection][key] = {};
      }
      var val = String(item[key]);
      if (!indices[collection][key][val]) {
        indices[collection][key][val] = [];
      }
      indices[collection][key][val].push(item.id);
    }
    notify(collection, "insert", item);
  }

  function find(collection, field, value) {
    var idx = indices[collection] && indices[collection][field];
    if (!idx) return [];
    var ids = idx[String(value)] || [];
    var results = [];
    for (var i = 0; i < ids.length; i++) {
      var item = collections[collection][ids[i]];
      if (item) results.push(item);
    }
    return results;
  }

  function getById(collection, id) {
    return collections[collection] ? collections[collection][id] : null;
  }

  function subscribe(callback) {
    subscribers.push(callback);
  }

  function notify(collection, action, item) {
    for (var i = 0; i < subscribers.length; i++) {
      subscribers[i]({collection: collection, action: action, item: item});
    }
  }

  function getStats() {
    var stats = {};
    var names = Object.keys(collections);
    for (var i = 0; i < names.length; i++) {
      stats[names[i]] = Object.keys(collections[names[i]]).length;
    }
    return stats;
  }

  return {
    createCollection: createCollection,
    insert: insert,
    find: find,
    getById: getById,
    subscribe: subscribe,
    getStats: getStats
  };
}

function populateDataStore(store) {
  store.createCollection("users");
  store.createCollection("posts");
  store.createCollection("comments");

  // Subscribe to changes
  var changeCount = 0;
  store.subscribe(function(event) {
    changeCount++;
  });

  // Insert users
  for (var u = 0; u < 50; u++) {
    store.insert("users", {
      id: u,
      name: "User " + u,
      role: ["admin", "editor", "viewer"][u % 3],
      active: u % 5 !== 0
    });
  }

  // Insert posts
  for (var p = 0; p < 100; p++) {
    store.insert("posts", {
      id: p,
      authorId: p % 50,
      title: "Post " + p,
      category: ["tech", "science", "art", "news", "sports"][p % 5],
      published: p % 3 !== 0
    });
  }

  // Insert comments
  for (var c = 0; c < 200; c++) {
    store.insert("comments", {
      id: c,
      postId: c % 100,
      authorId: c % 50,
      text: "Comment " + c + " on post " + (c % 100)
    });
  }

  return changeCount;
}

// ============================================================
// Full startup simulation
// ============================================================

function simulateStartup() {
  // Phase 1: Configuration
  var config = createConfig();

  // Phase 2: Event system
  var events = createEventSystem();
  var eventCount = registerEventListeners(events);

  // Phase 3: Router
  var router = createRouter();
  var routeCount = registerRoutes(router);

  // Phase 4: Component registry
  var registry = createComponentRegistry();
  var compCount = registerComponents(registry);

  // Phase 5: Data store
  var store = createDataStore();
  var changeCount = populateDataStore(store);

  // Phase 6: Fire startup events
  events.emit("app:init", {config: config});
  events.emit("app:ready", {timestamp: Date.now()});

  // Phase 7: Match some routes and create components
  var testUrls = [
    "/home", "/profile/42", "/settings/7/edit",
    "/messages/123/comments", "/feed", "/explore/99"
  ];
  var matchCount = 0;
  for (var i = 0; i < testUrls.length; i++) {
    var result = router.match(testUrls[i]);
    if (result) {
      matchCount++;
      // Create a component for each matched route
      registry.create("View", {route: result.route.path});
      registry.create("Text", {content: testUrls[i]});
    }
  }

  // Phase 8: Query the data store
  var queryResults = 0;
  var admins = store.find("users", "role", "admin");
  queryResults += admins.length;
  var techPosts = store.find("posts", "category", "tech");
  queryResults += techPosts.length;

  return {
    config: config.app.name.length,
    events: eventCount,
    routes: routeCount,
    components: compCount,
    dataChanges: changeCount,
    matches: matchCount,
    queries: queryResults
  };
}

// --- Benchmark: Full startup ---
function benchFullStartup() {
  var total = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var result = simulateStartup();
    total += result.events + result.routes + result.components +
             result.dataChanges + result.matches + result.queries;
  }
  return total;
}

// --- Benchmark: Config + Events only ---
function benchConfigEvents() {
  var total = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var config = createConfig();
    var events = createEventSystem();
    registerEventListeners(events);
    var results = events.emit("app:init", {config: config});
    total += results.length + config.i18n.supportedLocales.length;
  }
  return total;
}

// --- Benchmark: Router throughput ---
function benchRouterThroughput() {
  var router = createRouter();
  registerRoutes(router);
  var matched = 0;
  var testUrls = [
    "/home", "/profile/42", "/settings/7/edit", "/messages/99/comments",
    "/feed", "/explore/1", "/admin/5", "/search/100/edit",
    "/notifications/3", "/create/88/comments"
  ];
  for (var i = 0; i < ITERATIONS * 20; i++) {
    var url = testUrls[i % testUrls.length];
    var result = router.match(url);
    if (result) matched++;
  }
  return matched;
}

// --- Benchmark: Component creation throughput ---
function benchComponentCreation() {
  var registry = createComponentRegistry();
  registerComponents(registry);
  var names = [
    "Button", "Text", "View", "Image", "Card",
    "Header", "Icon", "Label", "Badge", "Link"
  ];
  var total = 0;
  for (var i = 0; i < ITERATIONS * 5; i++) {
    var name = names[i % names.length];
    var comp = registry.create(name, {key: i, label: "item-" + i});
    comp.mount();
    total += comp.state.mounted ? 1 : 0;
  }
  return total;
}

// --- Benchmark: Data store operations ---
function benchDataStore() {
  var total = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var store = createDataStore();
    populateDataStore(store);
    // Run queries
    var admins = store.find("users", "role", "admin");
    var techPosts = store.find("posts", "category", "tech");
    var user5 = store.getById("users", 5);
    total += admins.length + techPosts.length + (user5 ? 1 : 0);
  }
  return total;
}

// Run all benchmarks
function runBench(name, fn) {
  var start = Date.now();
  var result = fn();
  var elapsed = Date.now() - start;
  var opsPerSec = Math.round(ITERATIONS / (elapsed / 1000));
  print("RESULT: " + name + " " + opsPerSec + " ops/sec");
  return result;
}

runBench("full_startup", benchFullStartup);
runBench("config_events", benchConfigEvents);
runBench("router_throughput", benchRouterThroughput);
runBench("component_creation", benchComponentCreation);
runBench("data_store", benchDataStore);
