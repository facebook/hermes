// Copyright 2006-2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This is a JavaScript implementation of the Richards
// benchmark from:
//
//    http://www.cl.cam.ac.uk/~mr10/Bench.html
//
// The benchmark was originally implemented in BCPL by
// Martin Richards.

const COUNT = 1000;

const EXPECTED_QUEUE_COUNT = 2322;
const EXPECTED_HOLD_COUNT = 928;

const ID_IDLE = 0;
const ID_WORKER = 1;
const ID_HANDLER_A = 2;
const ID_HANDLER_B = 3;
const ID_DEVICE_A = 4;
const ID_DEVICE_B = 5;
const NUMBER_OF_IDS = 6;

const KIND_DEVICE = 0;
const KIND_WORK = 1;

const STATE_RUNNING = 0;
const STATE_RUNNABLE = 1;
const STATE_SUSPENDED = 2;
const STATE_HELD = 4;
const STATE_SUSPENDED_RUNNABLE = STATE_SUSPENDED | STATE_RUNNABLE;
const STATE_NOT_HELD = ~STATE_HELD;

const DATA_SIZE = 4;

class Packet {
  link: ?Packet;
  id: number;
  kind: number;
  a1: number;
  a2: number[];

  constructor(link: ?Packet, id: number, kind: number) {
    this.link = link;
    this.id = id;
    this.kind = kind;
    this.a1 = 0;
    this.a2 = [0, 0, 0, 0];
  }

  addTo(queue: ?Packet): Packet {
    this.link = null;
    if (queue === null) return this;
    var next: Packet = queue;
    while (next.link !== null)
      next = next.link;
    next.link = this;
    return queue;
  }

  toString(): string {
    return "Packet";
  }
}

class Task {
  scheduler: Scheduler;

  constructor(scheduler: Scheduler) {
    this.scheduler = scheduler;
  }

  run(packet: ?Packet): ?TaskControlBlock {
    throw new Error("abstract");
  }

  toString(): string {
    return "Task";
  }
}

class TaskControlBlock {
  link: ?TaskControlBlock;
  id: number;
  priority: number;
  queue: ?Packet;
  state: number;
  task: Task;

  constructor(
    link: ?TaskControlBlock,
    id: number,
    priority: number,
    queue: ?Packet,
    task: Task
  ) {
    this.link = link;
    this.id = id;
    this.priority = priority;
    this.queue = queue;
    this.task = task;
    if (queue === null) {
      this.state = STATE_SUSPENDED;
    } else {
      this.state = STATE_SUSPENDED_RUNNABLE;
    }
  }

  setRunning(): void {
    this.state = STATE_RUNNING;
  }

  markAsNotHeld(): void {
    this.state = this.state & STATE_NOT_HELD;
  }

  markAsHeld(): void {
    this.state = this.state | STATE_HELD;
  }

  isHeldOrSuspended(): boolean {
    return (this.state & STATE_HELD) !== 0 || (this.state === STATE_SUSPENDED);
  }

  markAsSuspended(): void {
    this.state = this.state | STATE_SUSPENDED;
  }

  markAsRunnable(): void {
    this.state = this.state | STATE_RUNNABLE;
  }

  run(): ?TaskControlBlock {
    var packet: ?Packet;
    if (this.state === STATE_SUSPENDED_RUNNABLE) {
      packet = this.queue;
      if (packet === null) throw new Error("unreachable");
      this.queue = packet.link;
      if (this.queue === null) {
        this.state = STATE_RUNNING;
      } else {
        this.state = STATE_RUNNABLE;
      }
    } else {
      packet = null;
    }
    return this.task.run(packet);
  }

  checkPriorityAdd(task: TaskControlBlock, packet: Packet): TaskControlBlock {
    if (this.queue === null) {
      this.queue = packet;
      this.markAsRunnable();
      if (this.priority > task.priority) return this;
    } else {
      this.queue = packet.addTo(this.queue);
    }
    return task;
  }

  toString(): string {
    return "tcb { " + this.task.toString() + "@" + String(this.state) + " }";
  }
}

class IdleTask extends Task {
  v1: number;
  count: number;

  constructor(scheduler: Scheduler, v1: number, count: number) {
    super(scheduler);
    this.v1 = v1;
    this.count = count;
  }

  run(packet: ?Packet): ?TaskControlBlock {
    this.count--;
    if (this.count === 0) return this.scheduler.holdCurrent();
    if ((this.v1 & 1) === 0) {
      this.v1 = this.v1 >> 1;
      return this.scheduler.release(ID_DEVICE_A);
    } else {
      this.v1 = (this.v1 >> 1) ^ 0xD008;
      return this.scheduler.release(ID_DEVICE_B);
    }
  }

  toString(): string {
    return "IdleTask";
  }
}

class DeviceTask extends Task {
  v1: ?Packet;

  constructor(scheduler: Scheduler) {
    super(scheduler);
    this.v1 = null;
  }

  run(packet: ?Packet): ?TaskControlBlock {
    if (packet === null) {
      if (this.v1 === null) return this.scheduler.suspendCurrent();
      var v: Packet = this.v1;
      this.v1 = null;
      return this.scheduler.queue(v);
    } else {
      this.v1 = packet;
      return this.scheduler.holdCurrent();
    }
  }

  toString(): string {
    return "DeviceTask";
  }
}

class WorkerTask extends Task {
  v1: number;
  v2: number;

  constructor(scheduler: Scheduler, v1: number, v2: number) {
    super(scheduler);
    this.v1 = v1;
    this.v2 = v2;
  }

  run(packet: ?Packet): ?TaskControlBlock {
    if (packet === null) {
      return this.scheduler.suspendCurrent();
    } else {
      if (this.v1 === ID_HANDLER_A) {
        this.v1 = ID_HANDLER_B;
      } else {
        this.v1 = ID_HANDLER_A;
      }
      packet.id = this.v1;
      packet.a1 = 0;
      for (var i = 0; i < DATA_SIZE; i++) {
        this.v2++;
        if (this.v2 > 26) this.v2 = 1;
        packet.a2[i] = this.v2;
      }
      return this.scheduler.queue(packet);
    }
  }

  toString(): string {
    return "WorkerTask";
  }
}

class HandlerTask extends Task {
  v1: ?Packet;
  v2: ?Packet;

  constructor(scheduler: Scheduler) {
    super(scheduler);
    this.v1 = null;
    this.v2 = null;
  }

  run(packet: ?Packet): ?TaskControlBlock {
    if (packet !== null) {
      if (packet.kind === KIND_WORK) {
        this.v1 = packet.addTo(this.v1);
      } else {
        this.v2 = packet.addTo(this.v2);
      }
    }
    if (this.v1 !== null) {
      var count = this.v1.a1;
      var v: Packet;
      if (count < DATA_SIZE) {
        if (this.v2 !== null) {
          v = this.v2;
          this.v2 = this.v2.link;
          v.a1 = this.v1.a2[count];
          this.v1.a1 = count + 1;
          return this.scheduler.queue(v);
        }
      } else {
        v = this.v1;
        this.v1 = this.v1.link;
        return this.scheduler.queue(v);
      }
    }
    return this.scheduler.suspendCurrent();
  }

  toString(): string {
    return "HandlerTask";
  }
}

class Scheduler {
  queueCount: number;
  holdCount: number;
  blocks: (?TaskControlBlock)[];
  list: ?TaskControlBlock;
  currentTcb: ?TaskControlBlock;
  currentId: number;

  constructor() {
    this.queueCount = 0;
    this.holdCount = 0;
    this.blocks = [null, null, null, null, null, null];
    this.list = null;
    this.currentTcb = null;
    this.currentId = 0;
  }

  addIdleTask(id: number, priority: number, queue: ?Packet, count: number): void {
    this.addRunningTask(id, priority, queue, new IdleTask(this, 1, count));
  }

  addWorkerTask(id: number, priority: number, queue: Packet): void {
    this.addTask(id, priority, queue, new WorkerTask(this, ID_HANDLER_A, 0));
  }

  addHandlerTask(id: number, priority: number, queue: Packet): void {
    this.addTask(id, priority, queue, new HandlerTask(this));
  }

  addDeviceTask(id: number, priority: number, queue: ?Packet): void {
    this.addTask(id, priority, queue, new DeviceTask(this));
  }

  addRunningTask(id: number, priority: number, queue: ?Packet, task: Task): void {
    this.addTask(id, priority, queue, task);
    var tcb = this.currentTcb;
    if (tcb === null) throw new Error("unreachable");
    tcb.setRunning();
  }

  addTask(id: number, priority: number, queue: ?Packet, task: Task): void {
    this.currentTcb = new TaskControlBlock(this.list, id, priority, queue, task);
    this.list = this.currentTcb;
    this.blocks[id] = this.currentTcb;
  }

  schedule(): void {
    var tcb = this.list;
    while (tcb !== null) {
      this.currentTcb = tcb;
      if (tcb.isHeldOrSuspended()) {
        tcb = tcb.link;
      } else {
        this.currentId = tcb.id;
        tcb = tcb.run();
      }
    }
  }

  release(id: number): ?TaskControlBlock {
    var tcb = this.blocks[id];
    if (tcb === null) return tcb;
    tcb.markAsNotHeld();
    var cur = this.currentTcb;
    if (cur === null) throw new Error("unreachable");
    if (tcb.priority > cur.priority) {
      return tcb;
    } else {
      return cur;
    }
  }

  holdCurrent(): ?TaskControlBlock {
    this.holdCount++;
    var tcb = this.currentTcb;
    if (tcb === null) throw new Error("unreachable");
    tcb.markAsHeld();
    return tcb.link;
  }

  suspendCurrent(): TaskControlBlock {
    var tcb = this.currentTcb;
    if (tcb === null) throw new Error("unreachable");
    tcb.markAsSuspended();
    return tcb;
  }

  queue(packet: Packet): ?TaskControlBlock {
    var t = this.blocks[packet.id];
    if (t === null) return t;
    this.queueCount++;
    packet.link = null;
    packet.id = this.currentId;
    var cur = this.currentTcb;
    if (cur === null) throw new Error("unreachable");
    return t.checkPriorityAdd(cur, packet);
  }
}

function runRichards(): void {
  var scheduler = new Scheduler();
  scheduler.addIdleTask(ID_IDLE, 0, null, COUNT);

  var queue = new Packet(null, ID_WORKER, KIND_WORK);
  queue = new Packet(queue, ID_WORKER, KIND_WORK);
  scheduler.addWorkerTask(ID_WORKER, 1000, queue);

  queue = new Packet(null, ID_DEVICE_A, KIND_DEVICE);
  queue = new Packet(queue, ID_DEVICE_A, KIND_DEVICE);
  queue = new Packet(queue, ID_DEVICE_A, KIND_DEVICE);
  scheduler.addHandlerTask(ID_HANDLER_A, 2000, queue);

  queue = new Packet(null, ID_DEVICE_B, KIND_DEVICE);
  queue = new Packet(queue, ID_DEVICE_B, KIND_DEVICE);
  queue = new Packet(queue, ID_DEVICE_B, KIND_DEVICE);
  scheduler.addHandlerTask(ID_HANDLER_B, 3000, queue);

  scheduler.addDeviceTask(ID_DEVICE_A, 4000, null);
  scheduler.addDeviceTask(ID_DEVICE_B, 5000, null);

  scheduler.schedule();

  if (scheduler.queueCount !== EXPECTED_QUEUE_COUNT ||
      scheduler.holdCount !== EXPECTED_HOLD_COUNT) {
    var msg =
        "Error during execution: queueCount = " + String(scheduler.queueCount) +
        ", holdCount = " + String(scheduler.holdCount) + ".";
    throw new Error(msg);
  }
}

var t1: number = Date.now();
for (var i = 0; i < 400; i++) {
  runRichards();
}
console.log(Date.now() - t1, "ms");
