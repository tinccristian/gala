#include "first_app.h"


// std
#include <stdexcept>
#include <array>

namespace gala
{
	FirstApp::FirstApp()
	{
		loadModels();
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(galaDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run()
	{
		while (!galaWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(galaDevice.device());
	}

	void FirstApp::sierpinski(
		std::vector<GalaModel::Vertex>& vertices,
		int depth,
		glm::vec2 left,
		glm::vec2 right,
		glm::vec2 top) {
		if (depth <= 0) {
			vertices.push_back({ top });
			vertices.push_back({ right });
			vertices.push_back({ left });
		}
		else {
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			sierpinski(vertices, depth - 1, left, leftRight, leftTop);
			sierpinski(vertices, depth - 1, leftRight, right, rightTop);
			sierpinski(vertices, depth - 1, leftTop, rightTop, top);
		}
	}

	void FirstApp::loadModels() {
		//std::vector<GalaModel::Vertex> vertices{
		//	{{ 0.0f,-0.5f}},
		//	{{ 0.5f, 0.5f}},
		//	{{-0.5f, 0.5f}}
		//};

		//galaModel = std::make_unique<GalaModel>(galaDevice, vertices);

		std::vector<GalaModel::Vertex> vertices{};
		sierpinski(vertices, 5, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, -0.5f });
		galaModel = std::make_unique<GalaModel>(galaDevice, vertices);

	}

	void FirstApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(galaDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		GalaPipeline::defaultPipelineConfigInfo(
			pipelineConfig,
			galaSwapChain.width(),
			galaSwapChain.height());
		pipelineConfig.renderPass = galaSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		galaPipeline = std::make_unique<GalaPipeline>(
			galaDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}


	void FirstApp::createCommandBuffers()
	{
		commandBuffers.resize(galaSwapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = galaDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(galaDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
		for (int i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffers!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = galaSwapChain.getRenderPass();
			renderPassInfo.framebuffer = galaSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = galaSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
			clearValues[1].depthStencil = {1.0f, 0};
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			galaPipeline->bind(commandBuffers[i]);
			galaModel->bind(commandBuffers[i]);
			galaModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}
	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = galaSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = galaSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if(result!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

}