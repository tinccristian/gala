#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace gala {
	class GalaWindow 
	{
		public:
			GalaWindow(int width, int height, std:: string name);
			~GalaWindow();

			GalaWindow(const GalaWindow&) = delete;
			GalaWindow &operator=(const GalaWindow&) = delete;


			bool shouldClose() { return glfwWindowShouldClose(window); }
			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
			bool wasWindowResized() { return frameBufferResized; }
			void resetWindowResizedFlag() { frameBufferResized = false; }


			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		private:
			static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
			void initWindow();

			int width;
			int height;
			bool frameBufferResized = false;

			std::string windowName;
			GLFWwindow* window;
	};
}