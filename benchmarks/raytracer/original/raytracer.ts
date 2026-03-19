
class Vector {
    x: number;
    y: number;
    z: number;
    constructor(x: number, y: number, z: number) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    static times(k: number, v: Vector) { return new Vector(k * v.x, k * v.y, k * v.z); }
    static minus(v1: Vector, v2: Vector) { return new Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z); }
    static plus(v1: Vector, v2: Vector) { return new Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z); }
    static dot(v1: Vector, v2: Vector) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
    static mag(v: Vector) { return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
    static norm(v: Vector) {
        var mag = Vector.mag(v);
        var div = (mag === 0) ? Infinity : 1.0 / mag;
        return Vector.times(div, v);
    }
    static cross(v1: Vector, v2: Vector) {
        return new Vector(v1.y * v2.z - v1.z * v2.y,
                          v1.z * v2.x - v1.x * v2.z,
                          v1.x * v2.y - v1.y * v2.x);
    }
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
    static scale(k: number, v: Color) { return new Color(k * v.r, k * v.g, k * v.b); }
    static plus(v1: Color, v2: Color) { return new Color(v1.r + v2.r, v1.g + v2.g, v1.b + v2.b); }
    static times(v1: Color, v2: Color) { return new Color(v1.r * v2.r, v1.g * v2.g, v1.b * v2.b); }
    static white = new Color(1.0, 1.0, 1.0);
    static grey = new Color(0.5, 0.5, 0.5);
    static black = new Color(0.0, 0.0, 0.0);
    static background = Color.black;
    static defaultColor = Color.black;
    static toDrawingColor(c: Color) {
        var legalize = d => d > 1 ? 1 : d;
        return {
            r: Math.floor(legalize(c.r) * 255),
            g: Math.floor(legalize(c.g) * 255),
            b: Math.floor(legalize(c.b) * 255)
        }
    }
}

class Camera {
    forward: Vector;
    right: Vector;
    up: Vector;
    pos: Vector;

    constructor(pos: Vector, lookAt: Vector) {
        this.pos = pos;
        var down = new Vector(0.0, -1.0, 0.0);
        this.forward = Vector.norm(Vector.minus(lookAt, this.pos));
        this.right = Vector.times(1.5, Vector.norm(Vector.cross(this.forward, down)));
        this.up = Vector.times(1.5, Vector.norm(Vector.cross(this.forward, this.right)));
    }
}

// Interfaces removed — hermes -parse-ts does not support standalone interface
// declarations. Type annotations on class fields and function parameters are
// kept as they are supported and provide optimization hints.

class Sphere {
    radius2: number;
    center: Vector;
    surface: Surface;

    constructor(center: Vector, radius: number, surface: Surface) {
        this.center = center;
        this.surface = surface;
        this.radius2 = radius * radius;
    }
    normal(pos: Vector): Vector { return Vector.norm(Vector.minus(pos, this.center)); }
    intersect(ray: Ray) {
        var eo = Vector.minus(this.center, ray.start);
        var v = Vector.dot(eo, ray.dir);
        var dist = 0;
        if (v >= 0) {
            var disc = this.radius2 - (Vector.dot(eo, eo) - v * v);
            if (disc >= 0) {
                dist = v - Math.sqrt(disc);
            }
        }
        if (dist === 0) {
            return null;
        } else {
            return { thing: this, ray: ray, dist: dist };
        }
    }
}

class Plane {
    normal: (pos: Vector) => Vector;
    intersect: (ray: Ray) => Intersection;
    surface: Surface;
    constructor(norm: Vector, offset: number, surface: Surface) {
        this.surface = surface;
        this.normal = function(pos: Vector) { return norm; }
        this.intersect = function(ray: Ray): Intersection {
            var denom = Vector.dot(norm, ray.dir);
            if (denom > 0) {
                return null;
            } else {
                var dist = (Vector.dot(norm, ray.start) + offset) / (-denom);
                return { thing: this, ray: ray, dist: dist };
            }
        }
    }
}

var Surfaces = {
    shiny: {
        diffuse: function(pos) { return Color.white; },
        specular: function(pos) { return Color.grey; },
        reflect: function(pos) { return 0.7; },
        roughness: 250
    },
    checkerboard: {
        diffuse: function(pos) {
            if ((Math.floor(pos.z) + Math.floor(pos.x)) % 2 !== 0) {
                return Color.white;
            } else {
                return Color.black;
            }
        },
        specular: function(pos) { return Color.white; },
        reflect: function(pos) {
            if ((Math.floor(pos.z) + Math.floor(pos.x)) % 2 !== 0) {
                return 0.1;
            } else {
                return 0.7;
            }
        },
        roughness: 150
    }
};


class RayTracer {
    private maxDepth = 5;

    private intersections(ray: Ray, scene: Scene) {
        var closest = +Infinity;
        var closestInter: Intersection = undefined;
        for (var i in scene.things) {
            var inter = scene.things[i].intersect(ray);
            if (inter != null && inter.dist < closest) {
                closestInter = inter;
                closest = inter.dist;
            }
        }
        return closestInter;
    }

    private testRay(ray: Ray, scene: Scene) {
        var isect = this.intersections(ray, scene);
        if (isect != null) {
            return isect.dist;
        } else {
            return undefined;
        }
    }

    private traceRay(ray: Ray, scene: Scene, depth: number): Color {
        var isect = this.intersections(ray, scene);
        if (isect === undefined) {
            return Color.background;
        } else {
            return this.shade(isect, scene, depth);
        }
    }

    private shade(isect: Intersection, scene: Scene, depth: number) {
        var d = isect.ray.dir;
        var pos = Vector.plus(Vector.times(isect.dist, d), isect.ray.start);
        var normal = isect.thing.normal(pos);
        var reflectDir = Vector.minus(d, Vector.times(2, Vector.times(Vector.dot(normal, d), normal)));
        var naturalColor = Color.plus(Color.background,
                                      this.getNaturalColor(isect.thing, pos, normal, reflectDir, scene));
        var reflectedColor = (depth >= this.maxDepth) ? Color.grey : this.getReflectionColor(isect.thing, pos, normal, reflectDir, scene, depth);
        return Color.plus(naturalColor, reflectedColor);
    }

    private getReflectionColor(thing: Thing, pos: Vector, normal: Vector, rd: Vector, scene: Scene, depth: number) {
        return Color.scale(thing.surface.reflect(pos), this.traceRay({ start: pos, dir: rd }, scene, depth + 1));
    }

    private getNaturalColor(thing: Thing, pos: Vector, norm: Vector, rd: Vector, scene: Scene) {
        var addLight = (col, light) => {
            var ldis = Vector.minus(light.pos, pos);
            var livec = Vector.norm(ldis);
            var neatIsect = this.testRay({ start: pos, dir: livec }, scene);
            var isInShadow = (neatIsect === undefined) ? false : (neatIsect <= Vector.mag(ldis));
            if (isInShadow) {
                return col;
            } else {
                var illum = Vector.dot(livec, norm);
                var lcolor = (illum > 0) ? Color.scale(illum, light.color)
                                          : Color.defaultColor;
                var specular = Vector.dot(livec, Vector.norm(rd));
                var scolor = (specular > 0) ? Color.scale(Math.pow(specular, thing.surface.roughness), light.color)
                                          : Color.defaultColor;
                return Color.plus(col, Color.plus(Color.times(thing.surface.diffuse(pos), lcolor),
                                                  Color.times(thing.surface.specular(pos), scolor)));
            }
        }
        return scene.lights.reduce(addLight, Color.defaultColor);
    }

    render(scene, ctx, screenWidth, screenHeight) {
        var getPoint = (x, y, camera) => {
            var recenterX = x =>(x - (screenWidth / 2.0)) / 2.0 / screenWidth;
            var recenterY = y => - (y - (screenHeight / 2.0)) / 2.0 / screenHeight;
            return Vector.norm(Vector.plus(camera.forward, Vector.plus(Vector.times(recenterX(x), camera.right), Vector.times(recenterY(y), camera.up))));
        }
        for (var y = 0; y < screenHeight; y++) {
            for (var x = 0; x < screenWidth; x++) {
                var color = this.traceRay({ start: scene.camera.pos, dir: getPoint(x, y, scene.camera) }, scene, 0);
                var c = Color.toDrawingColor(color);
                ctx[y * screenWidth + x] = ((255 & 0xFF) << 24) | ((c.b & 0xFF) << 16) | ((c.g & 0xFF) << 8) | (c.r & 0xFF);
            }
        }
    }
}


function defaultScene(): Scene {
    return {
        things: [new Plane(new Vector(0.0, 1.0, 0.0), 0.0, Surfaces.checkerboard),
                 new Sphere(new Vector(0.0, 1.0, -0.25), 1.0, Surfaces.shiny),
                 new Sphere(new Vector(-1.0, 0.5, 1.5), 0.5, Surfaces.shiny)],
        lights: [{ pos: new Vector(-2.0, 2.5, 0.0), color: new Color(0.49, 0.07, 0.07) },
                 { pos: new Vector(1.5, 2.5, 1.5), color: new Color(0.07, 0.07, 0.49) },
                 { pos: new Vector(1.5, 2.5, -1.5), color: new Color(0.07, 0.49, 0.071) },
                 { pos: new Vector(0.0, 3.5, 0.0), color: new Color(0.21, 0.21, 0.35) }],
        camera: new Camera(new Vector(3.0, 2.0, 4.0), new Vector(-1.0, 0.5, 0.0))
    };
}

function exec() {
    let width = 256;
    let height = 256;
    var ctx = new Uint32Array(width * height);
    var rayTracer = new RayTracer();
    let t1 = Date.now();
    rayTracer.render(defaultScene(), ctx, width, height);
    print("exec time: " + (Date.now() - t1) + " ms");
}
exec();
