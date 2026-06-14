#pragma once

#include"myEngine-device.h"

namespace myEngine
{
	class Texture
	{
	public:
		

		Texture(Device& device, const uint32_t& texWidth, const uint32_t& texHeight, const VkImageType& imageType, const VkFormat& format,
			const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkSharingMode& sharingMode,
			const VkSampleCountFlagBits& sampleWay);
		~Texture()
		{
			vkDestroySampler(device.getLogicalDevice(), sampler, nullptr);
			vkDestroyImageView(device.getLogicalDevice(), textureImageView, nullptr);
			vkDestroyImage(device.getLogicalDevice(), textureImage, nullptr);
			vkFreeMemory(device.getLogicalDevice(), textureImageMemory, nullptr);
		}
		
		Texture(const Texture&) = delete;	
		Texture& operator=(const Texture&) = delete;

		void CreateSampler(VkFilter filterMode = VK_FILTER_LINEAR, VkSamplerAddressMode overSampleMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkBool32 isShadowMap = VK_FALSE, VkCompareOp shadowDepthCompareMode = VK_COMPARE_OP_NEVER)
		{
			this->createSampler(filterMode, overSampleMode, isShadowMap, shadowDepthCompareMode);
		}

		void createTextureImageView(VkFormat format= VK_FORMAT_R8G8B8A8_SRGB,VkImageAspectFlags imageFlag= VK_IMAGE_ASPECT_COLOR_BIT)
		{
			textureImageView = device.createImageView(textureImage, format, imageFlag);
		}

		VkDescriptorImageInfo descriptorImageInfoBuild( VkImageLayout imageLayout= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkImage getTextureImage() { return textureImage; }
		VkImageView getTextureImageView() { return textureImageView; }

	private:

		void createSampler(VkFilter filterMode , VkSamplerAddressMode overSampleMode, VkBool32 isShadowMap, VkCompareOp shadowDepthCompareMode );
		Device& device;

		VkSampler sampler;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
	};


}
