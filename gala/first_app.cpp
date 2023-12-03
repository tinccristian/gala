#include "first_app.h"

#include "movement_controller.h"
#include "gala_camera.h"
#include "simple_render_system.h"
#include "gala_buffer.h"

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

	struct GlobalUbo {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ -1.f,-3.f,-1.f });
	};

	float MAX_FRAME_RATE = 1000;

	FirstApp::FirstApp() { loadGameObjects(); }

	FirstApp::~FirstApp() {}

	void FirstApp::run() {

		std::vector<std::unique_ptr<GalaBuffer>> uboBuffers(GalaSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<GalaBuffer>(
				galaDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		SimpleRenderSystem simpleRenderSystem{ galaDevice, galaRenderer.getSwapChainRenderPass() };
		GalaCamera camera{};
		//camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
		camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = GalaGameObject::createGameObject();
		MovementController cameraController{};

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
				int frameIndex = galaRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera
				};

				//update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				//render
				galaRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				galaRenderer.endSwapChainRenderPass(commandBuffer);
				galaRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(galaDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<GalaModel> galaModel = GalaModel::createModelFromFile (galaDevice, "D:\\Projects\\gala\\gala\\models\\flat_vase.obj"); //check if obj is created

		auto flatVase = GalaGameObject::createGameObject();
		flatVase.model = galaModel;
		flatVase.transform.translation = { -.5f,.5f,2.5f };
		flatVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.push_back(std::move(flatVase));

		galaModel = GalaModel::createModelFromFile (galaDevice, "D:\\Projects\\gala\\gala\\models\\smooth_vase.obj"); //check if obj is created

		auto smoothVase = GalaGameObject::createGameObject();
		smoothVase.model = galaModel;
		smoothVase.transform.translation = { .0f,.5f,2.5f };
		smoothVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.push_back(std::move(smoothVase));
	}
}