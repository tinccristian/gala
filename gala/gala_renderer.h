#pragma once

#include "gala_window.h"
#include "gala_device.h"
#include "gala_swap_chain.h"

//std
#include <memory>
#include <vector>
#include <cassert>

namespace gala
{
	class GalaRenderer
	{
	public:
		GalaRenderer(GalaWindow& galaWindow, GalaDevice& galaDevice);
		~GalaRenderer();

		VkRenderPass getSwapChainRenderPass() const { return galaSwapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return currentFrameIndex;
		}

		GalaRenderer(const GalaRenderer&) = delete;
		GalaRenderer& operator=(const GalaRenderer&) = delete;

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		GalaWindow& galaWindow;
		GalaDevice& galaDevice;
		std::unique_ptr<GalaSwapChain> galaSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{ false };
	};
}