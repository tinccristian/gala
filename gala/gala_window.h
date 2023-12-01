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

			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

		private:

			void initWindow();

			const int width;
			const int height;

			std::string windowName;
			GLFWwindow* window;
	};
}