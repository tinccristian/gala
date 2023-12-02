#include "first_app.h"

#include "simple_render_system.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace gala {

FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() {}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{galaDevice, galaRenderer.getSwapChainRenderPass()};

  while (!galaWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = galaRenderer.beginFrame()) {
      galaRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      galaRenderer.endSwapChainRenderPass(commandBuffer);
      galaRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(galaDevice.device());
}

void FirstApp::loadGameObjects() {
  std::vector<GalaModel::Vertex> vertices{
      {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
  auto galaModel = std::make_shared<GalaModel>(galaDevice, vertices);

  auto triangle = GalaGameObject::createGameObject();
  triangle.model = galaModel;
  triangle.color = {.1f, .8f, .1f};
  triangle.transform2d.translation.x = .2f;
  triangle.transform2d.scale = {2.f, .5f};
  triangle.transform2d.rotation = .25f * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

}
