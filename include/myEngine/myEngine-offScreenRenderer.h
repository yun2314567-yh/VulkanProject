#pragma once
#include"myEngine-device.h"
#include"myEngine-Texture.h"
namespace myEngine
{
	class offScreenRenderer
	{
	public:
		offScreenRenderer(Device& device, VkExtent2D extent, bool isShadowPass = false);
		~offScreenRenderer()
		{
			vkDestroySampler(_device.getLogicalDevice(), sampler, nullptr);
			vkDestroyImageView(_device.getLogicalDevice(), depthImageView, nullptr);
			vkDestroyImage(_device.getLogicalDevice(), depthImage, nullptr);
			vkFreeMemory(_device.getLogicalDevice(), depthImageMem, nullptr);
			vkDestroyImageView(_device.getLogicalDevice(), colorImageView, nullptr);
			vkDestroyImage(_device.getLogicalDevice(), colorImage, nullptr);
			vkFreeMemory(_device.getLogicalDevice(), colorImageMem, nullptr);
			//vkDestroyFramebuffer(_device.getLogicalDevice(), _frameBuffer, nullptr);
			//vkDestroyRenderPass(_device.getLogicalDevice(), _renderPass, nullptr);
		}

		void setColorImage(std::unique_ptr<Texture> colorImage) { colorTex = std::move(colorImage); }
		void setDepthImage(std::unique_ptr<Texture> depthImage) { depthTex = std::move(depthImage); }
		//VkRenderPass getRenderPass() { return _renderPass; }
		VkImage getColorImage() { return colorTex->getTextureImage(); }
		VkFormat getColorImageFormat() { return colorTex->getTextureFormat(); }
		VkImage getDepthImage() { return depthTex->getTextureImage(); }
		VkFormat getDepthImageFormat() { return depthTex->getTextureFormat(); }
		VkDescriptorImageInfo getDepthImageInfo(VkImageLayout imageLayout) { return depthTex->descriptorImageInfoBuild(imageLayout); }
		VkDescriptorImageInfo getColorImageInfo(VkImageLayout imageLayout) { return colorTex->descriptorImageInfoBuild(imageLayout); }
		VkImageView getColorImageView() { return colorTex->getTextureImageView(); }
		VkImageView getDepthImageView() { return depthTex->getTextureImageView(); }
		VkExtent2D getExtent() { return _extent; }

		VkSampler getColorSampler() { return colorTex->getSampler(); }
		VkSampler getDepthSampler() { return depthTex->getSampler(); }

		void setAndCreateDepthImage(VkImageUsageFlags  useflags) { createDepthImage(useflags); }
		void setAndCreateSampler(VkFilter filter=VK_FILTER_LINEAR,
			VkSamplerAddressMode mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			VkBorderColor color = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			VkBool32 able = VK_FALSE,
			VkCompareOp op = VK_COMPARE_OP_ALWAYS)
		{
			createSampler(filter,mode, color, able, op);
		}


		void beginPass(VkCommandBuffer command);

		void endPass(VkCommandBuffer command);

	private:
		bool isShadowPass = false;
		void createRenderPass();
		void createColorImage();
		void createDepthImage(VkImageUsageFlags useflags);
		void createSampler(VkFilter filter,
			VkSamplerAddressMode mode ,
			VkBorderColor color ,
			VkBool32 able ,
			VkCompareOp op );

		Device& _device;

		VkExtent2D _extent;

		
		VkFormat depthFormat;

		std::unique_ptr<Texture> colorTex;
		std::unique_ptr<Texture> depthTex;

		VkImage colorImage;
		VkDeviceMemory colorImageMem;
		VkImageView colorImageView;

		VkImage depthImage;
		VkDeviceMemory depthImageMem;
		VkImageView depthImageView;

		VkSampler sampler;



		//VkRenderPass _renderPass=VK_NULL_HANDLE;
		//VkFramebuffer _frameBuffer=VK_NULL_HANDLE;
	};
}