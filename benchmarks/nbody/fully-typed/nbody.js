// @flow strict
'use strict';
(function main() {

/* The Computer Language Benchmarks Game
   https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
   contributed by Isaac Gouy
   modified by Andrey Filatkin */

const PI:number = Math.PI;
const SOLAR_MASS:number = 4 * PI * PI;
const DAYS_PER_YEAR = 365.24;

class Body {
  x: number;
  y: number;
  z: number;
  vx: number;
  vy: number;
  vz: number;
  mass: number;

  constructor(
    x: number,
    y: number,
    z: number,
    vx: number,
    vy: number,
    vz: number,
    mass: number
  ) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.vx = vx;
    this.vy = vy;
    this.vz = vz;
    this.mass = mass;
  }
}

function Jupiter(): Body {
  return new Body(
    4.8414314424647209,
    -1.16032004402742839,
    -1.03622044471123109e-1,
    1.66007664274403694e-3 * DAYS_PER_YEAR,
    7.69901118419740425e-3 * DAYS_PER_YEAR,
    -6.90460016972063023e-5 * DAYS_PER_YEAR,
    9.54791938424326609e-4 * SOLAR_MASS
  );
}

function Saturn(): Body {
  return new Body(
    8.34336671824457987,
    4.12479856412430479,
    -4.03523417114321381e-1,
    -2.76742510726862411e-3 * DAYS_PER_YEAR,
    4.99852801234917238e-3 * DAYS_PER_YEAR,
    2.30417297573763929e-5 * DAYS_PER_YEAR,
    2.85885980666130812e-4 * SOLAR_MASS
  );
}

function Uranus(): Body {
  return new Body(
    1.2894369562139131e1,
    -1.51111514016986312e1,
    -2.23307578892655734e-1,
    2.96460137564761618e-3 * DAYS_PER_YEAR,
    2.3784717395948095e-3 * DAYS_PER_YEAR,
    -2.96589568540237556e-5 * DAYS_PER_YEAR,
    4.36624404335156298e-5 * SOLAR_MASS
  );
}

function Neptune(): Body {
  return new Body(
    1.53796971148509165e1,
    -2.59193146099879641e1,
    1.79258772950371181e-1,
    2.68067772490389322e-3 * DAYS_PER_YEAR,
    1.62824170038242295e-3 * DAYS_PER_YEAR,
    -9.5159225451971587e-5 * DAYS_PER_YEAR,
    5.15138902046611451e-5 * SOLAR_MASS
  );
}

function Sun(): Body {
  return new Body(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, SOLAR_MASS);
}

const bodies: Body[] = [Sun(), Jupiter(), Saturn(), Uranus(), Neptune()];

function offsetMomentum(): void {
  let px = 0;
  let py = 0;
  let pz = 0;
  const size: number = bodies.length;
  for (let i = 0; i < size; i++) {
    const body:Body = bodies[i];
    const mass:number = body.mass;
    px += body.vx * mass;
    py += body.vy * mass;
    pz += body.vz * mass;
  }

  const body:Body = bodies[0];
  body.vx = -px / SOLAR_MASS;
  body.vy = -py / SOLAR_MASS;
  body.vz = -pz / SOLAR_MASS;
}

function advance(dt: number) {
  const size: number = bodies.length;

  for (let i: number = 0; i < size; i++) {
    const bodyi: Body = bodies[i];
    let vxi: number = bodyi.vx;
    let vyi: number = bodyi.vy;
    let vzi: number = bodyi.vz;
    for (let j: number = i + 1; j < size; j++) {
      const bodyj: Body = bodies[j];
      const dx: number = bodyi.x - bodyj.x;
      const dy: number = bodyi.y - bodyj.y;
      const dz: number = bodyi.z - bodyj.z;

      const d2: number = dx * dx + dy * dy + dz * dz;
      const mag: number = dt / (d2 * Math.sqrt(d2));

      const massj: number = bodyj.mass;
      vxi -= dx * massj * mag;
      vyi -= dy * massj * mag;
      vzi -= dz * massj * mag;

      const massi: number = bodyi.mass;
      bodyj.vx += dx * massi * mag;
      bodyj.vy += dy * massi * mag;
      bodyj.vz += dz * massi * mag;
    }
    bodyi.vx = vxi;
    bodyi.vy = vyi;
    bodyi.vz = vzi;
  }

  for (let i: number = 0; i < size; i++) {
    const body: Body = bodies[i];
    body.x += dt * body.vx;
    body.y += dt * body.vy;
    body.z += dt * body.vz;
  }
}

function energy(): number {
  let e = 0;
  const size: number = bodies.length;

  for (let i = 0; i < size; i++) {
    const bodyi: Body = bodies[i];

    e +=
      0.5 *
      bodyi.mass *
      (bodyi.vx * bodyi.vx + bodyi.vy * bodyi.vy + bodyi.vz * bodyi.vz);

    for (let j = i + 1; j < size; j++) {
      const bodyj: Body = bodies[j];
      const dx: number = bodyi.x - bodyj.x;
      const dy: number = bodyi.y - bodyj.y;
      const dz: number = bodyi.z - bodyj.z;

      const distance: number = Math.sqrt(dx * dx + dy * dy + dz * dz);
      e -= (bodyi.mass * bodyj.mass) / distance;
    }
  }
  return e;
}

function nbody(): number {
  const n = 400_000;
  //const n = 100;

  offsetMomentum();
  for (let i = 0; i < n; i++) {
    advance(0.01);
  }

  return energy();
}
print(nbody());

})();
