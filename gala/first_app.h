#pragma once

#include "gala_window.h"
#include "gala_pipeline.h"
#include "gala_device.h"
#include "gala_swap_chain.h"
#include "gala_model.h"

//std
#include <memory>
#include <vector>

namespace gala 
{
	class FirstApp 
	{
		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			FirstApp();
			~FirstApp();

			FirstApp(const FirstApp&) = delete;
			FirstApp &operator=(const FirstApp&) = delete;

			void run();

		private:
			void loadModels();
			void createPipelineLayout();
			void createPipeline();
			void createCommandBuffers();
			void drawFrame();

			GalaWindow galaWindow{ WIDTH,HEIGHT,"Hello Vulkan!" };
			GalaDevice galaDevice{ galaWindow };
			GalaSwapChain galaSwapChain{ galaDevice, galaWindow.getExtent()};
			std::unique_ptr<GalaPipeline> galaPipeline;
			VkPipelineLayout pipelineLayout;
			std::vector<VkCommandBuffer> commandBuffers;
			std::unique_ptr<GalaModel> galaModel;
	};
}