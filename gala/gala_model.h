#pragma once

#include "gala_device.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace gala {
class GalaModel {
 public:
  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  GalaModel(GalaDevice &device, const std::vector<Vertex> &vertices);
  ~GalaModel();

  GalaModel(const GalaModel &) = delete;
  GalaModel &operator=(const GalaModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

 private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);

  GalaDevice &galaDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};
}
