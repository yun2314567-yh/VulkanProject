#pragma once
#include<memory>
#include"myEngine-device.h"


namespace myEngine
{
	

	class SwapChain
	{
	public:

		SwapChain(Device& device,VkExtent2D windowExtent);
		SwapChain(Device& device,VkExtent2D windowExtent,std::shared_ptr<SwapChain> oldSwapChain);

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;

		~SwapChain()
		{
			vkDeviceWaitIdle(device.getLogicalDevice());

			for (auto& fb : swapChainFrameBuffers)
				vkDestroyFramebuffer(device.getLogicalDevice(), fb, nullptr);

			vkDestroyRenderPass(device.getLogicalDevice(), _renderPass, nullptr);
			vkDestroyImageView(device.getLogicalDevice(), depthImageView, nullptr);
			vkDestroyImage(device.getLogicalDevice(), depthImage, nullptr);
			vkFreeMemory(device.getLogicalDevice(), depthMemory, nullptr);

			for (auto& iv : swapChainImageViews)
				vkDestroyImageView(device.getLogicalDevice(), iv, nullptr);

			for (size_t i = 0; i < MAX_FRAMEBUFFERS_IN_SAMETIME; i++)
			{
				vkDestroySemaphore(device.getLogicalDevice(), readyForPresenting[i], nullptr);
				vkDestroySemaphore(device.getLogicalDevice(), readyForRendering[i], nullptr);
				vkDestroyFence(device.getLogicalDevice(), inFlightFences[i], nullptr);

			}

			vkDestroySwapchainKHR(device.getLogicalDevice(), _swapChain, nullptr);
		}
		VkSemaphore readyForRender(const int index) { return readyForRendering[index]; }
		VkSemaphore readyForPresent(const int index) { return readyForPresenting[index]; }

		float extentAspectRatio() { return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height); }
		VkRenderPass getRenderPass() { return _renderPass; }
		VkFramebuffer getFrameBuffer(const int index) { return swapChainFrameBuffers[index]; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		size_t getImageCount() { return swapChainImages.size(); }

		VkImage getImage(const int index) { return swapChainImages[index]; }
		VkFormat getImageFormat() { return swapChainImageFormat; }	

		VkFence getCurrentFence() { return inFlightFences[currentFrame]; }

		VkImageView getImageView(const int index) { return swapChainImageViews[index]; }
		VkImageView getDepthImageView() { return depthImageView; }

		VkResult acquireNextImage(uint32_t* imageIndex);

		VkResult submitCommandBuffers(const VkCommandBuffer* commandBuffers, uint32_t* imageIndex);
		
		bool compareSwapFormat(const SwapChain& other) const
		{
			return swapChainImageFormat == other.swapChainImageFormat&& depthFormat == other.depthFormat;
		}

	private:
		Device& device;
		VkExtent2D windowExtent;
		void createSwapChain();
		void createImageViews();
		void createSyncObjects();
		void createDepthBuffer();
		void createRenderPass();
		void createFrameBuffers();

		int currentFrame = 0;	

		std::shared_ptr<SwapChain> oldSwapChain;

		VkSwapchainKHR _swapChain;
		VkRenderPass _renderPass;

		std::vector<VkFramebuffer> swapChainFrameBuffers;

		std::vector<VkSemaphore> readyForRendering;
		std::vector<VkSemaphore> readyForPresenting;

		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imageFences;

		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkImage> swapChainImages;
		
		VkFormat swapChainImageFormat;
		VkFormat depthFormat;
		VkExtent2D swapChainExtent;

		VkImage depthImage;
		VkImageView depthImageView;
		VkDeviceMemory depthMemory;

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats); 
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	};
}