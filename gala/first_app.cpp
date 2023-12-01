#include "first_app.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>

namespace gala
{
	struct SimplePushConstantData
	{											 // ( 1 , 0 )
		glm::mat2 transform{1.f}; //identity matrix ( 0 , 1 )
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp()
	{
		loadGameObjects();
		createPipelineLayout();
		recreateSwapChain();
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

	//void FirstApp::loadGameObjects() {
	//	std::vector<GalaModel::Vertex> vertices{
	//		{{ 0.0f,-0.5f}, {1.0f,0.0f,0.0f}},
	//		{{ 0.5f, 0.5f}, {0.0f,1.0f,0.0f}},
	//		{{-0.5f, 0.5f}, {0.0f,0.0f,1.0f}}
	//	};
	//	//sierpinski(vertices, 5, { -0.5f, 0.5f }, { 0.5f, 0.5f }, { 0.0f, -0.5f });

	//	auto galaModel = std::make_shared<GalaModel>(galaDevice, vertices);
	//	auto triangle = GalaGameObject::createGameObject();
	//	triangle.model = galaModel;
	//	triangle.color = { .1f,.8f,.1f };
	//	triangle.transform2d.translation.x = .2f;
	//	triangle.transform2d.scale = { 2.f, .5f };
	//	triangle.transform2d.rotation = .25f * glm::two_pi<float>();

	//	gameObjects.push_back(std::move(triangle));
	//}

	void FirstApp::loadGameObjects() {
		std::vector<GalaModel::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };
		auto lveModel = std::make_shared<GalaModel>(galaDevice, vertices);

		// https://www.color-hex.com/color-palette/5361
		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}  //
		};
		for (auto& color : colors) {
			color = glm::pow(color, glm::vec3{ 2.2f });
		}
		for (int i = 0; i < 40; i++) {
			auto triangle = GalaGameObject::createGameObject();
			triangle.model = lveModel;
			triangle.transform2d.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
			triangle.color = colors[i % colors.size()];
			gameObjects.push_back(std::move(triangle));
		}
	}
	void FirstApp::createPipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset	 = 0;
		pushConstantRange.size		 = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(galaDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}


	void FirstApp::recreateSwapChain()
	{
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
			galaSwapChain = std::make_unique<GalaSwapChain>(galaDevice, extent, std::move(galaSwapChain));
			if (galaSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		// if render pass compatible do nothing else
		createPipeline();
	}

	void FirstApp::createPipeline()
	{
		assert(galaSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		GalaPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = galaSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		galaPipeline = std::make_unique<GalaPipeline>(
			galaDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			galaDevice.device(), 
			galaDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void FirstApp::createCommandBuffers(){
		commandBuffers.resize(galaSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = galaDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());


		if (vkAllocateCommandBuffers(galaDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}

	}

	void FirstApp::recordCommandBuffer(int imageIndex){
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffers!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass  = galaSwapChain->getRenderPass();
		renderPassInfo.framebuffer = galaSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = galaSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color           = { 0.01f, 0.01f, 0.01f, 0.1f };
		clearValues[1].depthStencil    = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues    = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float>(galaSwapChain->getSwapChainExtent().width);
		viewport.height   = static_cast<float>(galaSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, galaSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer)
	{
		//update
		int i = 0;
		for (auto& obj : gameObjects) {
			i += 1;
			obj.transform2d.rotation =
				glm::mod<float>(obj.transform2d.rotation + 0.0001f * i, 2.f * glm::pi<float>());
		}



		// render
		galaPipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			//obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.001f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}

	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		auto result = galaSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		recordCommandBuffer(imageIndex);
		result = galaSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || galaWindow.wasWindowResized()) 
		{
			galaWindow.resetWindowResizedFlag();
			return;
		}
		if(result!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

}