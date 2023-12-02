#include "gala_window.h"

// std
#include <stdexcept>

namespace gala {
	GalaWindow::GalaWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	GalaWindow::~GalaWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void GalaWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void GalaWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to craete window surface");
		}
	}

	void GalaWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto galaWindow = reinterpret_cast<GalaWindow*>(glfwGetWindowUserPointer(window));
		galaWindow->framebufferResized = true;
		galaWindow->width = width;
		galaWindow->height = height;
	}
}