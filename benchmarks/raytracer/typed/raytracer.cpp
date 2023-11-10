/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>

#if USE_GUI
#include "libgui/gui.h"

#include "libgui/sokol/sokol_app.h"
#include "libgui/sokol/sokol_gfx.h"
#include "libgui/sokol/sokol_time.h"

#include "libgui/cimgui/imgui/imgui.h"
#include "libgui/sokol/sokol_imgui.h"
#else
#define SOKOL_IMPL
#include "libgui/sokol/sokol_time.h"

#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000
#define IM_COL32(R, G, B, A)                                                   \
  (((uint32_t)(A) << IM_COL32_A_SHIFT) | ((uint32_t)(B) << IM_COL32_B_SHIFT) | \
   ((uint32_t)(G) << IM_COL32_G_SHIFT) | ((uint32_t)(R) << IM_COL32_R_SHIFT))
#endif

class Vector {
 public:
  double x, y, z;

  Vector(double x, double y, double z) : x(x), y(y), z(z) {}

  static Vector times(double k, const Vector &v) {
    return Vector(k * v.x, k * v.y, k * v.z);
  }

  static Vector minus(const Vector &v1, const Vector &v2) {
    return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
  }

  static Vector plus(const Vector &v1, const Vector &v2) {
    return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
  }

  static double dot(const Vector &v1, const Vector &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  static double mag(const Vector &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  }

  static Vector norm(const Vector &v) {
    double magnitude = Vector::mag(v);
    double div = (magnitude == 0) ? INFINITY : 1.0f / magnitude;
    return Vector::times(div, v);
  }

  static Vector cross(const Vector &v1, const Vector &v2) {
    return Vector(
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x);
  }
};

class Color {
 public:
  double r, g, b;

  Color(double r, double g, double b) : r(r), g(g), b(b) {}

  static Color scale(double k, const Color &v) {
    return Color(k * v.r, k * v.g, k * v.b);
  }

  static Color plus(const Color &v1, const Color &v2) {
    return Color(v1.r + v2.r, v1.g + v2.g, v1.b + v2.b);
  }

  static Color times(const Color &v1, const Color &v2) {
    return Color(v1.r * v2.r, v1.g * v2.g, v1.b * v2.b);
  }

  static Color white() {
    return Color(1.0f, 1.0f, 1.0f);
  }
  static Color grey() {
    return Color(0.5f, 0.5f, 0.5f);
  }
  static Color black() {
    return Color(0.0f, 0.0f, 0.0f);
  }
  static Color background() {
    return black();
  }
  static Color defaultColor() {
    return black();
  }

  static uint32_t toDrawingColor(const Color &c) {
    auto legalize = [](double d) { return d > 1 ? 1 : d; };
    return IM_COL32(
        std::floor(legalize(c.r) * 255),
        std::floor(legalize(c.g) * 255),
        std::floor(legalize(c.b) * 255),
        255);
  }
};

static Vector s_down(0.0f, -1.0f, 0.0f);
class Camera {
 public:
  Vector pos;
  Vector forward;
  Vector right;
  Vector up;

  Camera(const Vector &pos, const Vector &lookAt)
      : pos(pos),
        forward(Vector::norm(Vector::minus(lookAt, this->pos))),
        right(Vector::times(
            1.5f,
            Vector::norm(Vector::cross(this->forward, s_down)))),
        up(Vector::times(
            1.5f,
            Vector::norm(Vector::cross(this->forward, this->right)))) {}
};

class Ray {
 public:
  Vector start;
  Vector dir;

  Ray(const Vector &start, const Vector &dir) : start(start), dir(dir) {}
};

class Thing;

// Assuming Thing class is defined elsewhere
class Intersection {
 public:
  Thing *thing; // Using pointer for reference semantics
  Ray ray;
  double dist;

  Intersection(Thing *thing, const Ray &ray, double dist)
      : thing(thing), ray(ray), dist(dist) {}
};

class Surface {
 public:
  std::function<Color(const Vector &)> diffuse;
  std::function<Color(const Vector &)> specular;
  std::function<double(const Vector &)> reflect;
  double roughness;

  Surface(
      std::function<Color(const Vector &)> diffuse,
      std::function<Color(const Vector &)> specular,
      std::function<double(const Vector &)> reflect,
      double roughness)
      : diffuse(diffuse),
        specular(specular),
        reflect(reflect),
        roughness(roughness) {}
};

class Thing {
 public:
  Surface surface;

  explicit Thing(const Surface &surface) : surface(surface) {}

  virtual std::unique_ptr<Intersection> intersect(const Ray &ray) {
    throw std::logic_error("intersect method is virtual");
  }

  virtual Vector normal(const Vector &pos) {
    throw std::logic_error("normal method is virtual");
  }
};

class Light {
 public:
  Vector pos;
  Color color;

  Light(const Vector &pos, const Color &color) : pos(pos), color(color) {}
};

class Scene {
 public:
  std::vector<std::shared_ptr<Thing>> things;
  std::vector<std::unique_ptr<Light>> lights;
  std::unique_ptr<Camera> camera;
};

class Sphere : public Thing {
 public:
  Vector center;
  double radius2;

  Sphere(const Vector &center, double radius, const Surface &surface)
      : Thing(surface), center(center), radius2(radius * radius) {}

  Vector normal(const Vector &pos) override {
    return Vector::norm(Vector::minus(pos, center));
  }

  std::unique_ptr<Intersection> intersect(const Ray &ray) override {
    Vector eo = Vector::minus(center, ray.start);
    double v = Vector::dot(eo, ray.dir);
    double dist = 0;
    if (v >= 0) {
      double disc = radius2 - (Vector::dot(eo, eo) - v * v);
      if (disc >= 0) {
        dist = v - std::sqrt(disc);
      }
    }
    if (dist == 0) {
      return nullptr;
    } else {
      return std::make_unique<Intersection>(this, ray, dist);
    }
  }
};

class Plane : public Thing {
 public:
  Vector norm;
  double offset;

  Plane(const Vector &norm, double offset, const Surface &surface)
      : Thing(surface), norm(norm), offset(offset) {}

  Vector normal(const Vector &pos) override {
    return norm;
  }

  std::unique_ptr<Intersection> intersect(const Ray &ray) override {
    double denom = Vector::dot(norm, ray.dir);
    if (denom > 0) {
      return nullptr;
    } else {
      double dist = (Vector::dot(norm, ray.start) + offset) / -denom;
      return std::make_unique<Intersection>(this, ray, dist);
    }
  }
};

// Helper lambdas for shiny surface
auto shiny_diffuse = [](const Vector &pos) -> Color { return Color::white(); };
auto shiny_specular = [](const Vector &pos) -> Color { return Color::grey(); };
auto shiny_reflect = [](const Vector &pos) -> double { return 0.7f; };

// Shiny surface object
Surface Surfaces_shiny(shiny_diffuse, shiny_specular, shiny_reflect, 250);

// Helper lambdas for checkerboard surface
auto checkerboard_diffuse = [](const Vector &pos) -> Color {
  if (static_cast<int>(std::floor(pos.z)) % 2 !=
      static_cast<int>(std::floor(pos.x)) % 2) {
    return Color::white();
  } else {
    return Color::black();
  }
};

auto checkerboard_specular = [](const Vector &pos) -> Color {
  return Color::white();
};

auto checkerboard_reflect = [](const Vector &pos) -> double {
  if (static_cast<int>(std::floor(pos.z)) % 2 !=
      static_cast<int>(std::floor(pos.x)) % 2) {
    return 0.1f;
  } else {
    return 0.7f;
  }
};

// Checkerboard surface object
Surface Surfaces_checkerboard(
    checkerboard_diffuse,
    checkerboard_specular,
    checkerboard_reflect,
    150);

class RayTracer {
 private:
  int maxDepth;

 public:
  RayTracer() : maxDepth(5) {}

  std::unique_ptr<Intersection> intersections(
      const Ray &ray,
      const Scene &scene) {
    double closest = std::numeric_limits<double>::infinity();
    std::unique_ptr<Intersection> closestInter = nullptr;
    for (const auto &thing : scene.things) {
      auto inter = thing->intersect(ray);
      if (inter != nullptr && inter->dist < closest) {
        closestInter = std::move(inter);
        closest = closestInter->dist;
      }
    }
    return closestInter;
  }

  std::pair<bool, double> testRay(const Ray &ray, const Scene &scene) {
    auto isect = intersections(ray, scene);
    if (isect != nullptr) {
      return {true, isect->dist};
    } else {
      return {false, 0};
    }
  }

  Color traceRay(const Ray &ray, const Scene &scene, int depth) {
    auto isect = intersections(ray, scene);
    if (isect == nullptr) {
      return Color::background();
    } else {
      return shade(*isect, scene, depth);
    }
  }

  Color shade(const Intersection &isect, const Scene &scene, int depth) {
    Vector d = isect.ray.dir;
    Vector pos = Vector::plus(Vector::times(isect.dist, d), isect.ray.start);
    Vector normal = isect.thing->normal(pos);
    Vector reflectDir = Vector::minus(
        d, Vector::times(2, Vector::times(Vector::dot(normal, d), normal)));
    Color naturalColor = Color::plus(
        Color::background(),
        getNaturalColor(*isect.thing, pos, normal, reflectDir, scene));
    Color reflectedColor(0, 0, 0);
    if (depth >= maxDepth) {
      reflectedColor = Color::grey();
    } else {
      reflectedColor = getReflectionColor(
          *isect.thing, pos, normal, reflectDir, scene, depth);
    }
    return Color::plus(naturalColor, reflectedColor);
  }

  Color getReflectionColor(
      const Thing &thing,
      const Vector &pos,
      const Vector &normal,
      const Vector &rd,
      const Scene &scene,
      int depth) {
    return Color::scale(
        thing.surface.reflect(pos), traceRay(Ray(pos, rd), scene, depth + 1));
  }

  Color getNaturalColor(
      const Thing &thing,
      const Vector &pos,
      const Vector &norm,
      const Vector &rd,
      const Scene &scene) {
    auto addLight = [this, &thing, &pos, &norm, &rd, &scene](
                        Color col, const Light &light) {
      auto ldis = Vector::minus(light.pos, pos);
      auto livec = Vector::norm(ldis);
      auto neatIsect = testRay(Ray(pos, livec), scene);
      auto isInShadow =
          neatIsect.first && neatIsect.second <= Vector::mag(ldis);
      if (isInShadow) {
        return col;
      } else {
        auto illum = Vector::dot(livec, norm);
        auto lcolor = illum > 0 ? Color::scale(illum, light.color)
                                : Color::defaultColor();
        auto specular = Vector::dot(livec, Vector::norm(rd));
        auto scolor = specular > 0
            ? Color::scale(
                  std::pow(specular, thing.surface.roughness), light.color)
            : Color::defaultColor();
        return Color::plus(
            col,
            Color::plus(
                Color::times(thing.surface.diffuse(pos), lcolor),
                Color::times(thing.surface.specular(pos), scolor)));
      }
    };
    // return scene.lights.reduce(addLight, Color::defaultColor);
    Color result = Color::defaultColor();
    for (const auto &light : scene.lights) {
      result = addLight(result, *light);
    }
    return result;
  }

  void
  render(const Scene &scene, int screenWidth, int screenHeight, uint32_t *buf) {
    auto getPoint = [screenWidth, screenHeight](
                        int x, int y, const Camera &camera) {
      auto recenterX = [screenWidth](int x) {
        return (x - screenWidth / 2.0) / 2.0 / screenWidth;
      };
      auto recenterY = [screenHeight](int y) {
        return -(y - screenHeight / 2.0) / 2.0 / screenHeight;
      };
      return Vector::norm(Vector::plus(
          camera.forward,
          Vector::plus(
              Vector::times(recenterX(x), camera.right),
              Vector::times(recenterY(y), camera.up))));
    };
    for (int y = 0; y < screenHeight; ++y) {
      for (int x = 0; x < screenWidth; ++x) {
        Vector point = getPoint(x, y, *scene.camera);
        Color color = traceRay(Ray(scene.camera->pos, point), scene, 0);
        buf[y * screenWidth + x] = Color::toDrawingColor(color);
      }
    }
  }
};

Scene defaultScene() {
  Scene scene;

  scene.things.push_back(std::make_unique<Plane>(
      Vector(0.0, 1.0, 0.0), 0.0, Surfaces_checkerboard));
  scene.things.push_back(
      std::make_unique<Sphere>(Vector(0.0, 1.0, -0.25), 1.0, Surfaces_shiny));
  scene.things.push_back(
      std::make_unique<Sphere>(Vector(-1.0, 0.5, 1.5), 0.5, Surfaces_shiny));

  scene.lights.push_back(
      std::make_unique<Light>(Vector(-2.0, 2.5, 0.0), Color(0.49, 0.07, 0.07)));
  scene.lights.push_back(
      std::make_unique<Light>(Vector(1.5, 2.5, 1.5), Color(0.07, 0.07, 0.49)));
  scene.lights.push_back(
      std::make_unique<Light>(Vector(1.5, 2.5, -1.5), Color(0.07, 0.49, 0.07)));
  scene.lights.push_back(
      std::make_unique<Light>(Vector(0.0, 3.5, 0.0), Color(0.21, 0.21, 0.35)));

  scene.camera =
      std::make_unique<Camera>(Vector(3.0, 2.0, 4.0), Vector(-1.0, 0.5, 0.0));

  return scene;
}

void exec(int width, int height, uint32_t *buf) {
  RayTracer rayTracer;
  rayTracer.render(defaultScene(), width, height, buf);
}

uint32_t *render(int w, int h) {
  auto *buf = (uint32_t *)calloc(w * h * 4, 1);
  uint64_t t1 = stm_now();
  exec(w, h, buf);
  printf("exec time: %f ms\n", stm_ms(stm_since(t1)));
  return buf;
}

#if USE_GUI
void gui_on_main(sapp_desc *desc, int argc, char **argv) {
  desc->window_title = "C++ Raytracer";
  desc->width = 640;
  desc->height = 480;
}

static int s_imageHnd = -1;
static ImTextureID s_imageID = nullptr;

void gui_on_init() {
  int w = 256;
  int h = 256;
  auto *buf = render(w, h);
  s_imageHnd = make_image(w, h, (const unsigned char *)buf);
  s_imageID = simgui_imtextureid(*image_simgui_image(s_imageHnd));
  free(buf);
}

void gui_on_cleanup() {}

void gui_on_event(const sapp_event *ev) {}

void gui_on_frame(int width, int height, double secs) {
  ImGui::GetBackgroundDrawList()->AddImage(
      s_imageID,
      ImVec2(0, 0),
      ImVec2(width, height),
      ImVec2(0, 0),
      ImVec2(1, 1));
}
#else

int main(int argc, char **argv) {
  stm_setup();
  int w = 256;
  int h = 256;
  auto *buf = render(256, 256);
  free(buf);
}
#endif
