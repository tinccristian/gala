#pragma once

#include "gala_camera.h"
#include "gala_device.h"
#include "gala_frame_info.h"
#include "gala_game_object.h"
#include "gala_pipeline.h"

//std
#include <memory>
#include <vector>

namespace gala
{
	class SimpleRenderSystem
	{
	public:

		SimpleRenderSystem(GalaDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo, std::vector<GalaGameObject> &gameObjects);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		GalaDevice& galaDevice;

		std::unique_ptr<GalaPipeline> galaPipeline;
		VkPipelineLayout			  pipelineLayout;
	};
}