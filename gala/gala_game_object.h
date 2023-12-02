#pragma once

#include "gala_model.h"

// std
#include <memory>

namespace gala {

struct Transform2dComponent {
  glm::vec2 translation{};  // (position offset)
  glm::vec2 scale{1.f, 1.f};
  float rotation;

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMatrix{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMatrix * scaleMat;
  }
};

class GalaGameObject {
 public:
  using id_t = unsigned int;

  static GalaGameObject createGameObject() {
    static id_t currentId = 0;
    return GalaGameObject{currentId++};
  }

  GalaGameObject(const GalaGameObject &) = delete;
  GalaGameObject &operator=(const GalaGameObject &) = delete;
  GalaGameObject(GalaGameObject &&) = default;
  GalaGameObject &operator=(GalaGameObject &&) = default;

  id_t getId() { return id; }

  std::shared_ptr<GalaModel> model{};
  glm::vec3 color{};
  Transform2dComponent transform2d{};

 private:
  GalaGameObject(id_t objId) : id{objId} {}

  id_t id;
};
}
