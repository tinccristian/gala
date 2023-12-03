#include "gala_renderer.h"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace gala {
	GalaRenderer::GalaRenderer(GalaWindow& window, GalaDevice& device)
		: galaWindow{ window }, galaDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}

	GalaRenderer::~GalaRenderer() { freeCommandBuffers(); }

	void GalaRenderer::recreateSwapChain() {
		auto extent = galaWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = galaWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(galaDevice.device());

		if (galaSwapChain == nullptr) {
			galaSwapChain = std::make_unique<GalaSwapChain>(galaDevice, extent);
		}
		else {
			std::shared_ptr<GalaSwapChain> oldSwapChain = std::move(galaSwapChain);
			galaSwapChain = std::make_unique<GalaSwapChain>(galaDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*galaSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void GalaRenderer::createCommandBuffers() {
		commandBuffers.resize(GalaSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = galaDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(galaDevice.device(), &allocInfo, commandBuffers.data()) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void GalaRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			galaDevice.device(),
			galaDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer GalaRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");

		auto result = galaSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void GalaRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = galaSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			galaWindow.wasWindowResized()) {
			galaWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % GalaSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void GalaRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = galaSwapChain->getRenderPass();
		renderPassInfo.framebuffer = galaSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = galaSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(galaSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(galaSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, galaSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void GalaRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}
}