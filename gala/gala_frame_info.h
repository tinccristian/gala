#pragma once

#include "gala_camera.h"

//lib
#include <vulkan/vulkan.h>

namespace gala {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		GalaCamera& camera;
	};
}