#pragma once

#include "gala_device.h"
#include "gala_game_object.h"
#include "gala_renderer.h"
#include "gala_window.h"

// std
#include <memory>
#include <vector>

namespace gala {
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  GalaWindow galaWindow{ WIDTH, HEIGHT, "gala" };
  GalaDevice galaDevice{galaWindow};
  GalaRenderer galaRenderer{galaWindow, galaDevice};

  std::vector<GalaGameObject> gameObjects;
};
}
