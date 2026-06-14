#include"myEngine/myEngine-swapChain.h"
#include<array>
namespace myEngine
{
	SwapChain::SwapChain(Device& device, VkExtent2D windowExtent):device{device},windowExtent{windowExtent}
	{
		createSwapChain();
		createImageViews();
		createSyncObjects();
		createDepthBuffer();
		createRenderPass();
		createFrameBuffers();
	}

	SwapChain::SwapChain(Device& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> oldSwapChain) : device{ device }, windowExtent{ windowExtent }, oldSwapChain{ oldSwapChain }
	{
		createSwapChain();
		createImageViews();
		createSyncObjects();
		createDepthBuffer();
		createRenderPass();
		createFrameBuffers();
	}

	VkResult SwapChain::acquireNextImage(uint32_t* imageIndex)
	{
		vkWaitForFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), _swapChain, std::numeric_limits<uint64_t>::max(), readyForRendering[currentFrame], VK_NULL_HANDLE, imageIndex);

		return result;
	}

	VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* commandBuffers, uint32_t* imageIndex)
	{
		if(imageFences[*imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(device.getLogicalDevice(), 1, &imageFences[*imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

		imageFences[*imageIndex] = inFlightFences[currentFrame];

		VkSemaphore waitSemaphores[] = { readyForRendering[currentFrame] };
		VkPipelineStageFlags waitSemaphoresStagesMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSemaphore signalSemaphores[] = { readyForPresenting[currentFrame] };

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffers;
		submitInfo.pWaitDstStageMask = waitSemaphoresStagesMask;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

		if(vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("fail to submit draw command buffer");

		VkSwapchainKHR swapChains[] = { _swapChain };

		//提交交换链中的指定图像进行显示
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = imageIndex;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		auto result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAMEBUFFERS_IN_SAMETIME;
		return result;
	}

	void SwapChain::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = device.querySwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.features);

		uint32_t imageCount = swapChainSupport.features.minImageCount + 1;
		if (swapChainSupport.features.maxImageCount > 0 && imageCount > swapChainSupport.features.maxImageCount) {
			imageCount = swapChainSupport.features.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapChainInfo = {};
		swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainInfo.surface = device.getSurface();
		swapChainInfo.imageFormat = surfaceFormat.format;
		swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapChainInfo.imageExtent = extent;
		swapChainInfo.minImageCount = imageCount;
		swapChainInfo.imageArrayLayers = 1;
		swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndex indices = device.getQueueFamilyIndices();
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamilys,(uint32_t)indices.presentFamilys };

		if (indices.graphicsFamilys != indices.presentFamilys) {
			swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainInfo.queueFamilyIndexCount = 2;
			swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainInfo.queueFamilyIndexCount = 0;
			swapChainInfo.pQueueFamilyIndices = nullptr;
		}

		swapChainInfo.preTransform = swapChainSupport.features.currentTransform;
		swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainInfo.presentMode = presentMode;
		swapChainInfo.clipped = VK_TRUE;

		swapChainInfo.oldSwapchain = oldSwapChain==nullptr ? VK_NULL_HANDLE : oldSwapChain->_swapChain;



		if (vkCreateSwapchainKHR(device.getLogicalDevice(), &swapChainInfo, nullptr, &_swapChain) != VK_SUCCESS)
			throw std::runtime_error("交换链创建失败");

		vkGetSwapchainImagesKHR(device.getLogicalDevice(), _swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device.getLogicalDevice(), _swapChain, &imageCount, swapChainImages.data());
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		for (auto& image : swapChainImages)
		{
			device.transitionImageLayout(image, swapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	}

	void SwapChain::createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			swapChainImageViews[i] = device.createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}


	void SwapChain::createSyncObjects()
	{
		//信号量用于同步队列操作，栅栏用于同步渲染和程序本身
		readyForRendering.resize(MAX_FRAMEBUFFERS_IN_SAMETIME);
		readyForPresenting.resize(MAX_FRAMEBUFFERS_IN_SAMETIME);
		inFlightFences.resize(MAX_FRAMEBUFFERS_IN_SAMETIME);
		imageFences.resize(swapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMEBUFFERS_IN_SAMETIME; i++)
		{
			if (vkCreateSemaphore(device.getLogicalDevice(), &semaphoreInfo, nullptr, &readyForRendering[i]) != VK_SUCCESS || vkCreateSemaphore(device.getLogicalDevice(), &semaphoreInfo, nullptr, &readyForPresenting[i])
				|| vkCreateFence(device.getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
				throw std::runtime_error("同步对象创建失败");
		}

		
	}

	void SwapChain::createDepthBuffer()
	{
		depthFormat = device.findDepthFormat();



		device.createImage(swapChainExtent.width, swapChainExtent.height, VK_IMAGE_TYPE_2D, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT, depthImage, depthMemory);

		depthImageView = device.createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		device.transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	}

	void SwapChain::createRenderPass()
	{


		VkSubpassDependency subPassAttachment = {};
		subPassAttachment.srcSubpass = VK_SUBPASS_EXTERNAL;
		subPassAttachment.dstSubpass = 0;
		subPassAttachment.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subPassAttachment.srcAccessMask = 0;
		subPassAttachment.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subPassAttachment.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//颜色缓冲的操作
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//模板缓冲的操作
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = device.findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachRef = {};
		depthAttachRef.attachment = 1;
		depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachRef;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment,depthAttachment };

		VkRenderPassCreateInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderInfo.subpassCount = 1;
		renderInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderInfo.pAttachments = attachments.data();
		renderInfo.pSubpasses = &subpass;
		renderInfo.dependencyCount = 1;
		renderInfo.pDependencies = &subPassAttachment;

		if (vkCreateRenderPass(device.getLogicalDevice(), &renderInfo, nullptr, &_renderPass) != VK_SUCCESS)
			throw std::runtime_error("渲染通道创建失败");


	}

	void SwapChain::createFrameBuffers()
	{
		

		swapChainFrameBuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> attachments = { swapChainImageViews[i],depthImageView };

			VkFramebufferCreateInfo frameBufferInfo = {};
			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferInfo.width = swapChainExtent.width;
			frameBufferInfo.height = swapChainExtent.height;
			frameBufferInfo.pAttachments = attachments.data();
			frameBufferInfo.layers = 1;
			frameBufferInfo.renderPass = _renderPass;


			if (vkCreateFramebuffer(device.getLogicalDevice(), &frameBufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error(std::to_string(i) + "个帧缓冲创建失败");

		}
	}


	VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_R8G8B8A8_SRGB,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}


		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;//双缓冲


		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { //是否支持三重缓冲
				bestMode = availablePresentMode;
			}
			else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) { //是否支持无缓冲
				bestMode = availablePresentMode;
			}
		}
		return bestMode;
	}

	VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			int w, h;
			
			VkExtent2D actualExtent = windowExtent;
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}
	}

}