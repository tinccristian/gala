#pragma once

#include "gala_game_object.h"
#include "gala_window.h"

//libs
#define GLFW_CROSSHAIR_CURSOR   0x00036003
#define GLFW_HAND_CURSOR		0x00036004
#define GLFW_HRESIZE_CURSOR		0x00036005
#define GLFW_VRESIZE_CURSOR		0x00036006
#define GLFW_ARROW_CURSOR		0x00036001
#define GLFW_IBEAM_CURSOR		0x00036002

namespace gala {
	class MovementController {
	public:
		struct KeyMappings {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		};

		void moveInPlaneXZ(GLFWwindow* window, float dt, GalaGameObject& gameObject);

		void handleMouseInput(GLFWwindow* window, double mouseX, double mouseY);

		KeyMappings keys{};
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
		float mouseSensitivity{ 0.01f }; // Adjust sensitivity as needed
	};
}
