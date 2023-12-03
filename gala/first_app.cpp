#include "first_app.h"

#include "keyboard_movement_controller.h"
#include "gala_camera.h"
#include "simple_render_system.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>

namespace gala {
	float MAX_FRAME_RATE = 1000;

	FirstApp::FirstApp() { loadGameObjects(); }

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{ galaDevice, galaRenderer.getSwapChainRenderPass() };
		GalaCamera camera{};
		//camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
		camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = GalaGameObject::createGameObject();
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!galaWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			frameTime = glm::min(frameTime, MAX_FRAME_RATE);

			cameraController.moveInPlaneXZ(galaWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = galaRenderer.getAspectRatio();
			//camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);

			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = galaRenderer.beginFrame()) {
				galaRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				galaRenderer.endSwapChainRenderPass(commandBuffer);
				galaRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(galaDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<GalaModel> galaModel = GalaModel::createModelFromFile (galaDevice, "D:\\Projects\\gala\\gala\\models\\smooth_vase.obj");

		auto gameObject = GalaGameObject::createGameObject();
		gameObject.model = galaModel;
		gameObject.transform.translation = { .0f,.0f,2.5f };
		gameObject.transform.scale = glm::vec3(3.f);
		gameObjects.push_back(std::move(gameObject));
	}
}