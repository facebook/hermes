/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 */

function IM_COL32(r, g, b, a) {
  "inline";
  return ((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | (r & 0xFF);
}

class Vector {
  x: number;
  y: number;
  z: number;
  constructor(x: number, y: number, z: number) {
    this.x = x;
    this.y = y;
    this.z = z;
  }
}

function Vector_times(k: number, v: Vector) {
  return new Vector(k * v.x, k * v.y, k * v.z);
}
function Vector_minus(v1: Vector, v2: Vector) {
  return new Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}
function Vector_plus(v1: Vector, v2: Vector) {
  return new Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
function Vector_dot(v1: Vector, v2: Vector) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
function Vector_mag(v: Vector) {
  return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
function Vector_norm(v: Vector) {
  var mag = Vector_mag(v);
  var div = mag === 0 ? Infinity : 1.0 / mag;
  return Vector_times(div, v);
}
function Vector_cross(v1: Vector, v2: Vector) {
  return new Vector(
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x
  );
}

class Color {
  r: number;
  g: number;
  b: number;
  constructor(r: number, g: number, b: number) {
    this.r = r;
    this.g = g;
    this.b = b;
  }
}

function Color_scale(k: number, v: Color) {
  return new Color(k * v.r, k * v.g, k * v.b);
}
function Color_plus(v1: Color, v2: Color) {
  return new Color(v1.r + v2.r, v1.g + v2.g, v1.b + v2.b);
}
function Color_times(v1: Color, v2: Color) {
  return new Color(v1.r * v2.r, v1.g * v2.g, v1.b * v2.b);
}
const Color_white = new Color(1.0, 1.0, 1.0);
const Color_grey = new Color(0.5, 0.5, 0.5);
const Color_black = new Color(0.0, 0.0, 0.0);
const Color_background = Color_black;
const Color_defaultColor = Color_black;
function Color_toDrawingColor(c: Color) {
  var legalize = (d) => (d > 1 ? 1 : d);
  return {
    r: Math.floor(legalize(c.r) * 255),
    g: Math.floor(legalize(c.g) * 255),
    b: Math.floor(legalize(c.b) * 255),
  };
}

class Camera {
  pos: Vector;
  forward: Vector;
  right: Vector;
  up: Vector;

  constructor(pos: Vector, lookAt: Vector) {
    this.pos = pos;
    var down = new Vector(0.0, -1.0, 0.0);
    this.forward = Vector_norm(Vector_minus(lookAt, this.pos));
    this.right = Vector_times(
      1.5,
      Vector_norm(Vector_cross(this.forward, down))
    );
    this.up = Vector_times(
      1.5,
      Vector_norm(Vector_cross(this.forward, this.right))
    );
  }
}

class Ray {
  start: Vector;
  dir: Vector;

  constructor(start: Vector, dir: Vector) {
    this.start = start;
    this.dir = dir;
  }
}

class Intersection {
  thing: Thing;
  ray: Ray;
  dist: number;

  constructor(thing: Thing, ray: Ray, dist: number) {
    this.thing = thing;
    this.ray = ray;
    this.dist = dist;
  }
}

class Surface {
  diffuse: (pos: Vector) => Color;
  specular: (pos: Vector) => Color;
  reflect: (pos: Vector) => number;
  roughness: number;

  constructor(
    diffuse: (pos: Vector) => Color,
    specular: (pos: Vector) => Color,
    reflect: (pos: Vector) => number,
    roughness: number
  ) {
    this.diffuse = diffuse;
    this.specular = specular;
    this.reflect = reflect;
    this.roughness = roughness;
  }
}

class Thing {
  surface: Surface;

  constructor(
    surface: Surface
  ) {
    this.surface = surface;
  }

  intersect(ray: Ray): Intersection | null {
    throw TypeError("virtual intersect");
  }

  normal(pos: Vector): Vector {
    throw TypeError("virtual normal");
  }
}

class Light {
  pos: Vector;
  color: Color;

  constructor(pos: Vector, color: Color) {
    this.pos = pos;
    this.color = color;
  }
}

class Scene {
  things: Thing[];
  lights: Light[];
  camera: Camera;

  constructor(things: Thing[], lights: Light[], camera: Camera) {
    this.things = things;
    this.lights = lights;
    this.camera = camera;
  }
}

class Sphere extends Thing {
  center: Vector;
  radius2: number;

  constructor(center: Vector, radius: number, surface: Surface) {
    super(surface);
    this.center = center;
    this.radius2 = radius * radius;
  }
  normal(pos: Vector): Vector {
    return Vector_norm(Vector_minus(pos, this.center));
  }
  intersect(ray: Ray): Intersection | null {
    var eo = Vector_minus(this.center, ray.start);
    var v = Vector_dot(eo, ray.dir);
    var dist = 0;
    if (v >= 0) {
      var disc = this.radius2 - (Vector_dot(eo, eo) - v * v);
      if (disc >= 0) {
        dist = v - Math.sqrt(disc);
      }
    }
    if (dist === 0) {
      return null;
    } else {
      return new Intersection(this, ray, dist);
    }
  }
}

class Plane extends Thing {
  norm: Vector;
  offset: number;
  constructor(norm: Vector, offset: number, surface: Surface) {
    super(surface);
    this.norm = norm;
    this.offset = offset;
  }

  normal(pos: Vector): Vector {
    return this.norm;
  };
  intersect(ray: Ray): Intersection | null {
    var denom = Vector_dot(this.norm, ray.dir);
    if (denom > 0) {
      return null;
    } else {
      var dist = (Vector_dot(this.norm, ray.start) + this.offset) / -denom;
      return new Intersection(this, ray, dist);
    }
  };
}

var Surfaces_shiny: Surface = new Surface(
  function (pos): Color {
    return Color_white;
  },
  function (pos): Color {
    return Color_grey;
  },
  function (pos): number {
    return 0.7;
  },
  250
);
var Surfaces_checkerboard: Surface = new Surface(
  function (pos): Color {
    if ((Math.floor(pos.z) + Math.floor(pos.x)) % 2 !== 0) {
      return Color_white;
    } else {
      return Color_black;
    }
  },
  function (pos): Color {
    return Color_white;
  },
  function (pos): number {
    if ((Math.floor(pos.z) + Math.floor(pos.x)) % 2 !== 0) {
      return 0.1;
    } else {
      return 0.7;
    }
  },
  150
);

class RayTracer {
  maxDepth: number;

  constructor() {
    this.maxDepth = 5;
  }

  intersections(ray: Ray, scene: Scene) {
    var closest = +Infinity;
    var closestInter: ?Intersection = undefined;
    for (var i = 0; i < scene.things.length; ++i) {
      var inter: Intersection | null = scene.things[i].intersect(ray);
      if (inter != null) {
        var interAny: any = inter;
        var inter2: Intersection = interAny;
        if (inter2.dist < closest) {
          closestInter = inter2;
          closest = inter2.dist;
        }
      }
    }
    return closestInter;
  }

  testRay(ray: Ray, scene: Scene) {
    var isect = this.intersections(ray, scene);
    if (isect != null) {
      return isect.dist;
    } else {
      return undefined;
    }
  }

  traceRay(ray: Ray, scene: Scene, depth: number): Color {
    var isect = this.intersections(ray, scene);
    if (isect === undefined) {
      return Color_background;
    } else {
      return this.shade(isect, scene, depth);
    }
  }

  shade(isect: Intersection, scene: Scene, depth: number) {
    var d = isect.ray.dir;
    var pos = Vector_plus(Vector_times(isect.dist, d), isect.ray.start);
    var normal = isect.thing.normal(pos);
    var reflectDir = Vector_minus(
      d,
      Vector_times(2, Vector_times(Vector_dot(normal, d), normal))
    );
    var naturalColor = Color_plus(
      Color_background,
      this.getNaturalColor(isect.thing, pos, normal, reflectDir, scene)
    );
    var reflectedColor =
      depth >= this.maxDepth
        ? Color_grey
        : this.getReflectionColor(
            isect.thing,
            pos,
            normal,
            reflectDir,
            scene,
            depth
          );
    return Color_plus(naturalColor, reflectedColor);
  }

  getReflectionColor(
    thing: Thing,
    pos: Vector,
    normal: Vector,
    rd: Vector,
    scene: Scene,
    depth: number
  ) {
    return Color_scale(
      thing.surface.reflect(pos),
      this.traceRay(new Ray(pos, rd), scene, depth + 1)
    );
  }

  getNaturalColor(
    thing: Thing,
    pos: Vector,
    norm: Vector,
    rd: Vector,
    scene: Scene
  ): Color {
    var addLight = (col, light) => {
      var ldis = Vector_minus(light.pos, pos);
      var livec = Vector_norm(ldis);
      var neatIsect = this.testRay(new Ray(pos, livec), scene);
      var isInShadow =
        neatIsect === undefined ? false : neatIsect <= Vector_mag(ldis);
      if (isInShadow) {
        return col;
      } else {
        var illum = Vector_dot(livec, norm);
        var lcolor =
          illum > 0 ? Color_scale(illum, light.color) : Color_defaultColor;
        var specular = Vector_dot(livec, Vector_norm(rd));
        var scolor =
          specular > 0
            ? Color_scale(
                Math.pow(specular, thing.surface.roughness),
                light.color
              )
            : Color_defaultColor;
        return Color_plus(
          col,
          Color_plus(
            Color_times(thing.surface.diffuse(pos), lcolor),
            Color_times(thing.surface.specular(pos), scolor)
          )
        );
      }
    };
    // return scene.lights.reduce(addLight, Color_defaultColor);
    var result: Color = Color_defaultColor;
    for (var i = 0; i < scene.lights.length; ++i) {
      result = addLight(result, scene.lights[i]);
    }
    return result;
  }

  render(scene: Scene, screenWidth: number, screenHeight: number, buf: c_ptr) {
    var getPoint = (x, y, camera) => {
      var recenterX = (x) => (x - screenWidth / 2.0) / 2.0 / screenWidth;
      var recenterY = (y) => -(y - screenHeight / 2.0) / 2.0 / screenHeight;
      return Vector_norm(
        Vector_plus(
          camera.forward,
          Vector_plus(
            Vector_times(recenterX(x), camera.right),
            Vector_times(recenterY(y), camera.up)
          )
        )
      );
    };
    for (var y = 0; y < screenHeight; y++) {
      for (var x = 0; x < screenWidth; x++) {
        var color = this.traceRay(
          new Ray(scene.camera.pos, getPoint(x, y, scene.camera)),
          scene,
          0
        );
        var c = Color_toDrawingColor(color);
        _sh_ptr_write_c_uint(buf, 4 * (y * screenWidth + x), IM_COL32(c.r, c.g, c.b, 255));
      }
    }
  }
}

function defaultScene(): Scene {
  return {
    things: [
      new Plane(new Vector(0.0, 1.0, 0.0), 0.0, Surfaces_checkerboard),
      new Sphere(new Vector(0.0, 1.0, -0.25), 1.0, Surfaces_shiny),
      new Sphere(new Vector(-1.0, 0.5, 1.5), 0.5, Surfaces_shiny),
    ],
    lights: [
      { pos: new Vector(-2.0, 2.5, 0.0), color: new Color(0.49, 0.07, 0.07) },
      { pos: new Vector(1.5, 2.5, 1.5), color: new Color(0.07, 0.07, 0.49) },
      { pos: new Vector(1.5, 2.5, -1.5), color: new Color(0.07, 0.49, 0.071) },
      { pos: new Vector(0.0, 3.5, 0.0), color: new Color(0.21, 0.21, 0.35) },
    ],
    camera: new Camera(new Vector(3.0, 2.0, 4.0), new Vector(-1.0, 0.5, 0.0)),
  };
}

function exec(width: number, height: number, buf: c_ptr) {
  var rayTracer = new RayTracer();
  return rayTracer.render(defaultScene(), width, height, buf);
}

if (typeof performance === "undefined") {
  globalThis.performance = { now: Date.now };
}

if (typeof USE_GUI === "undefined") {
  let w = 256;
  let h = 256;
  let buf = allocTmp(w * h * 4);
  let t1 = performance.now();
  exec(w, h, buf);
  print("exec time: ", (performance.now() - t1), "ms");
  flushAllocTmp();
}
