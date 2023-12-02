#pragma once

#include "gala_device.h"
#include "gala_swap_chain.h"
#include "gala_window.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace gala {
    class GalaRenderer {

    public:
        GalaRenderer(GalaWindow& window, GalaDevice& device);
        ~GalaRenderer();

        GalaRenderer(const GalaRenderer&) = delete;
        GalaRenderer& operator=(const GalaRenderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const { return galaSwapChain->getRenderPass(); }
        float getAspectRatio() const {return galaSwapChain->extentAspectRatio();}
        bool isFrameInProgress() const { return isFrameStarted; 
        }

    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
        return commandBuffers[currentFrameIndex];
        }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  GalaWindow &galaWindow;
  GalaDevice &galaDevice;
  std::unique_ptr<GalaSwapChain> galaSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};
}
