#include "gala_model.h"

//std
#include<cassert>

namespace gala
{

	GalaModel::GalaModel(GalaDevice &device, const std::vector<Vertex> &vertices) :galaDevice{ device } {
		createVertexBuffer(vertices);
	}
	GalaModel::~GalaModel() {
		vkDestroyBuffer(galaDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(galaDevice.device(), vertexBufferMemory, nullptr);
	}

	void GalaModel::createVertexBuffer(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		galaDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexBufferMemory
		);
		void* data;
		vkMapMemory(galaDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(galaDevice.device(), vertexBufferMemory);
	}

	void GalaModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void GalaModel::draw(VkCommandBuffer commandBuffer){
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> GalaModel::Vertex::getBindingDescriptions(){
		return { {0,sizeof(Vertex),VK_VERTEX_INPUT_RATE_VERTEX} };
	}



	std::vector<VkVertexInputAttributeDescription> GalaModel::Vertex::getAttributeDescriptions() {
		return{ {0,0,VK_FORMAT_R32G32_SFLOAT,0} };
	}

}