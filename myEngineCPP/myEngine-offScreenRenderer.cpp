#include"myEngine/myEngine-offScreenRenderer.h"
#include<array>
#include<cassert>
namespace myEngine
{
	offScreenRenderer:: offScreenRenderer(Device& device,VkExtent2D extent,bool isShadowPass):_device(device),_extent(extent),isShadowPass(isShadowPass){}
	

	void offScreenRenderer::beginPass(VkCommandBuffer command)
	{
		//assert(_renderPass != VK_NULL_HANDLE);
		//assert(_frameBuffer != VK_NULL_HANDLE);
	    
		

		VkClearValue clearValue[2] = { {0,1,0,1},{1.,0} };
		VkRect2D area = {};
		area.offset = { 0,0 };
		area.extent = _extent;

		
		
		VkRenderingAttachmentInfo depthAttachment = {};
		depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthAttachment.clearValue = clearValue[1];
      
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.imageView = depthTex->getTextureImageView();
		
		VkRenderingInfo renderInfo = {};
		renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderInfo.renderArea.offset = { 0,0 };
		renderInfo.renderArea.extent = _extent;
		
		
		renderInfo.pDepthAttachment = &depthAttachment;
		renderInfo.layerCount = 1;
		
		VkRenderingAttachmentInfo colorAttachment = {};
		std::cout << "激活shadow: " << isShadowPass << std::endl;
		if (isShadowPass==false)
		{
			
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.clearValue = clearValue[0];
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.imageView = colorTex->getTextureImageView();
			
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
	
}