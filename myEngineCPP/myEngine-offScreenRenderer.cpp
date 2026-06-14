#include"myEngine/myEngine-offScreenRenderer.h"
#include<array>
#include<cassert>
namespace myEngine
{
	offScreenRenderer:: offScreenRenderer(Device& device,VkExtent2D extent,VkFormat colorFormat,bool isShadowPass):_device(device),_extent(extent),colorFormat(colorFormat),isShadowPass(isShadowPass)
	{
		depthFormat = _device.findDepthFormat();
		if(isShadowPass==false)
		createColorImage();
		
		
		
		
	}

	void offScreenRenderer::beginPass(VkCommandBuffer command)
	{
		//assert(_renderPass != VK_NULL_HANDLE);
		//assert(_frameBuffer != VK_NULL_HANDLE);
	    
		

		VkClearValue clearValue[2] = { {0,1,0,1},{1.,0} };
		VkRect2D area = {};
		area.offset = { 0,0 };
		area.extent = _extent;

		/*VkRenderPassBeginInfo passBeginInfo = {};
		passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		passBeginInfo.clearValueCount = 2;
		passBeginInfo.pClearValues = clearValue;
		passBeginInfo.framebuffer = _frameBuffer;
		passBeginInfo.renderArea = area;
		passBeginInfo.renderPass = _renderPass;

		vkCmdBeginRenderPass(command, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);*/
		
		
		VkRenderingAttachmentInfo depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.clearValue = clearValue[1];
      
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.imageView = depthImageView;
		
		VkRenderingInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea.offset = { 0,0 };
		renderInfo.renderArea.extent = _extent;
		
		
		renderInfo.pDepthAttachment = &depthAttachment;
		renderInfo.layerCount = 1;
		
		VkRenderingAttachmentInfo colorAttachment = {};

		if (isShadowPass==false)
		{
			
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.clearValue = clearValue[0];
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.imageView = colorImageView;
			
			renderInfo.colorAttachmentCount = 1;
		  renderInfo.pColorAttachments = &colorAttachment;
		}
		else
		{
			renderInfo.colorAttachmentCount = 0;
			renderInfo.pColorAttachments = nullptr;
		}
		
		

		

		vkCmdBeginRendering(command, &renderInfo);

		VkViewport viewport = {
			0,0,
			_extent.width,_extent.height,
			0,1
		};

		vkCmdSetViewportWithCount(command, 1, &viewport);
		vkCmdSetScissorWithCount(command, 1, &area);

	}

	void offScreenRenderer::endPass(VkCommandBuffer command)
	{
		
		vkCmdEndRendering(command);


	}
	void offScreenRenderer::createColorImage()
	{
		_device.createImage(_extent.width, _extent.height, 
			VK_IMAGE_TYPE_2D, 
			colorFormat, 
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT,
			colorImage, 
			colorImageMem);


		colorImageView=_device.createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
			         
		_device.transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void offScreenRenderer::createDepthImage(VkImageUsageFlags useflags)
	{
		_device.createImage(_extent.width, _extent.height,
			VK_IMAGE_TYPE_2D, depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			useflags,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_SAMPLE_COUNT_1_BIT,
			depthImage,
			depthImageMem
		);

		depthImageView = _device.createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		
		if(useflags & VK_IMAGE_USAGE_SAMPLED_BIT)
		  _device.transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		else
			_device.transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	/*void offScreenRenderer::createRenderPass()
	{
		VkAttachmentDescription colorAttach = {};
		colorAttach.format = colorFormat;
		colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentDescription depthAttachment={};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorRef = {0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
		VkAttachmentReference depthRef = { 1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.pDepthStencilAttachment = &depthRef;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttach,depthAttachment };

		VkRenderPassCreateInfo passInfo = {};
		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passInfo.attachmentCount = attachments.size();
		passInfo.pAttachments = attachments.data();
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;
		
		if (vkCreateRenderPass(_device.getLogicalDevice(), &passInfo, nullptr, &_renderPass) != VK_SUCCESS)
			throw std::runtime_error("离屏Pass创建失败");



	}*/

	/*void offScreenRenderer::createFrameBuffer()
	{
		std::array<VkImageView, 2> imageViews = { colorImageView,depthImageView };

		VkFramebufferCreateInfo frameInfo = {};
		frameInfo.sType= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameInfo.width = _extent.width;
		frameInfo.height = _extent.height;
		frameInfo.attachmentCount = imageViews.size();
		frameInfo.pAttachments = imageViews.data();

		frameInfo.renderPass = _renderPass;
		frameInfo.layers = 1;

		if (vkCreateFramebuffer(_device.getLogicalDevice(), &frameInfo, nullptr, &_frameBuffer) != VK_SUCCESS)
			throw std::runtime_error("离屏Frame创建失败");
	}*/

	void offScreenRenderer::createSampler(VkFilter filter,
		VkSamplerAddressMode mode,
		VkBorderColor color,
		VkBool32 able,
		VkCompareOp op)
	{

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = mode;
		samplerInfo.addressModeV = mode;
		samplerInfo.addressModeW = mode;
		samplerInfo.anisotropyEnable = VK_FALSE;
		
		samplerInfo.borderColor = color;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = able;
		samplerInfo.compareOp = op;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0;

		samplerInfo.maxLod = 0;
		samplerInfo.minLod = 0;

		if (vkCreateSampler(_device.getLogicalDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("离屏采样器创建失败");
	}
}