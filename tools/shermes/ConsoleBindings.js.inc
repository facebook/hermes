R"((function() {
  "use strict";

  var tasks = [];
  var nextTaskID = 0;
  var curTime = 0;

  // Return the deadline of the next task, or -1 if there is no task.
  function peekMacroTask() {
    return tasks.length ? tasks[0].deadline : -1;
  }

  // Run the next task if it's time.
  // `tm` is the current time.
  function runMacroTask(tm) {
    curTime = tm;
    if (tasks.length && tasks[0].deadline <= tm) {
      var task = tasks.shift();
      task.fn.apply(undefined, task.args);
    }
  }

  function setTimeout(fn, ms = 0, ...args) {
    var id = nextTaskID++;
    var task = {id, fn, deadline: curTime + Math.max(0, ms | 0), args};
    // Insert the task in the sorted list.
    var i = 0;
    for (i = 0; i < tasks.length; ++i) {
      if (tasks[i].deadline > task.deadline) {
        break;
      }
    }
    tasks.splice(i, 0, task);
    return id;
  }

  function clearTimeout(id) {
    for (var i = 0; i < tasks.length; i++) {
      if (tasks[i].id === id) {
        tasks.splice(i, 1);
        break;
      }
    }
  }

  function setImmediate(fn, ...args) {
    return setTimeout(fn, 0, ...args);
  }
  function clearImmediate(id) {
    return clearTimeout(id);
  }

  globalThis.setTimeout = setTimeout;
  globalThis.clearTimeout = clearTimeout;
  globalThis.setImmediate = setImmediate;
  globalThis.clearImmediate = clearImmediate;

  return {peek: peekMacroTask, run: runMacroTask};
})();
)"
