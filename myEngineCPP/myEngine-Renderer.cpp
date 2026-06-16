
#include<stdexcept>
#include<cassert>
#include<array>
#include"myEngine/myEngine-Renderer.h"

namespace myEngine
{
	Renderer::Renderer(Window& window, Device& device) :window(window), device(device)
	{
		createCommandBuffers();
		recreateSwapChain();
	}

	void Renderer::createCommandBuffers()
	{
		commandBuffers.resize(MAX_FRAMEBUFFERS_IN_SAMETIME);
		
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
		if (vkAllocateCommandBuffers(device.getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("???????????????");
	}

	

	void Renderer::recreateSwapChain()
	{
		auto extent = window.getExtent2D();
		while(extent.width == 0 || extent.height == 0) {
			extent = window.getExtent2D();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.getLogicalDevice());

		if(swapChain==nullptr)
			swapChain = std::make_unique<SwapChain>(device,extent);
		else
		{
			std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
			swapChain = std::make_unique<SwapChain>(device,extent,oldSwapChain);
			if (!oldSwapChain->compareSwapFormat(*swapChain.get()))
				throw std::runtime_error("????????ormat?????);
		}

	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert(!isFrameStarted && "?????????");

		auto result = swapChain->acquireNextImage(&currentImageIndex);

		if(result==VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return VK_NULL_HANDLE;
		}
		
		if(result!=VK_SUCCESS && result!=VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("??????????????);
		
		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		

		if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("??????????????");

		return commandBuffer;
	}

	void Renderer::endFrame()
	{
		assert(isFrameStarted && "???????);
		auto commandBuffer = getCurrentCommandBuffer();
		if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("??????????????);
		auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		//std::cout << "Present result: " << result << std::endl;

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.isWindowResized())
		{
			window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if(result != VK_SUCCESS)
			throw std::runtime_error("??????????????);
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMEBUFFERS_IN_SAMETIME;
	}

	void Renderer::beginRenderPass(VkCommandBuffer commandBuffer)
	{
		
		assert(isFrameStarted && "???????);
		assert(commandBuffer == getCurrentCommandBuffer() && "?????????????????);

		

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 1.f,1.f,1.f,1.f };
		clearValues[1].depthStencil = { 1.f,0 };

		VkRenderingAttachmentInfo colorAttachment = {};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.clearValue = clearValues[0];
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.imageView = swapChain->getImageView(currentImageIndex);
		
		VkRenderingAttachmentInfo depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.clearValue = clearValues[1];
       
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.imageLayout= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.imageView = swapChain->getDepthImageView();


		VkRenderingInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea.offset = { 0,0 };
		renderInfo.renderArea.extent = swapChain->getSwapChainExtent();
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;
		renderInfo.pDepthAttachment = &depthAttachment;
		renderInfo.layerCount = 1;
		
		vkCmdBeginRendering(commandBuffer, &renderInfo);

		VkViewport viewport = {};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = swapChain->getSwapChainExtent();

		//std::cout << "extent: " << scissor.extent.width << " " << scissor.extent.height << std::endl;

		vkCmdSetViewportWithCount(commandBuffer, 1, &viewport);	
		vkCmdSetScissorWithCount(commandBuffer, 1, &scissor);
		//vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		//vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "???????);
		assert(commandBuffer == getCurrentCommandBuffer() && "?????????????????);

		
		vkCmdEndRendering(commandBuffer);
	}
}
