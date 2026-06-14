#pragma once

#include<cassert>
#include"myEngine-device.h"
#include"myEngine-swapChain.h"
#include"myEngine-Window.h"


namespace myEngine
{
	class Renderer
	{
	public:
		Renderer(Window& window, Device& device);

		~Renderer()
		{
			vkFreeCommandBuffers(device.getLogicalDevice(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
			commandBuffers.clear();
		}

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		float getAspectRatio()const { return swapChain->extentAspectRatio(); }
		VkRenderPass getSwapChainRenderPass() { return swapChain->getRenderPass(); }

		VkImage getSwapChainCurrentImage() { return swapChain->getImage(currentImageIndex); }
		VkFormat getSwapChainImageFormat() { return swapChain->getImageFormat(); }

		void beginRenderPass(VkCommandBuffer commandBuffer);
		void endRenderPass(VkCommandBuffer commandBuffer);

		uint32_t getCurrentFrameIndex() { return currentFrameIndex; }

		VkFence getSwapChainCurrentFence() { return swapChain->getCurrentFence(); }

		VkCommandBuffer beginFrame();
		void endFrame();

	private:
		VkCommandBuffer getCurrentCommandBuffer()
		{
			assert(isFrameStarted && "在帧未开始时不能获取当前指令缓冲区");
			return commandBuffers[currentFrameIndex];
		}

		void createCommandBuffers();
		void recreateSwapChain();
		Window& window;
		Device& device;
		
		uint32_t currentImageIndex;

		std::unique_ptr<SwapChain> swapChain;

		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentFrameIndex = 0;
		bool isFrameStarted = false;
	};
}