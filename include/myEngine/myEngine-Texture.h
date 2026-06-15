#pragma once

#include"myEngine-device.h"

namespace myEngine
{
	class Texture
	{
	public:
		

		Texture(Device& device, const uint32_t& texWidth, const uint32_t& texHeight, const VkImageType& imageType, const VkFormat format,
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

		void setSampler(VkFilter filterMode = VK_FILTER_LINEAR, VkSamplerAddressMode overSampleMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,VkBorderColor color= VK_BORDER_COLOR_INT_OPAQUE_BLACK, VkBool32 compareAble = VK_FALSE, VkCompareOp compareOp = VK_COMPARE_OP_NEVER)
		{
			this->createSampler(filterMode, overSampleMode, color,compareAble, compareOp);
		}

		void createTextureImageView(VkImageAspectFlags imageFlag= VK_IMAGE_ASPECT_COLOR_BIT)
		{
			textureImageView = device.createImageView(textureImage, textureFormat, imageFlag);
		}

		VkDescriptorImageInfo descriptorImageInfoBuild( VkImageLayout imageLayout= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkFormat getTextureFormat() { return textureFormat; }
		VkImage getTextureImage() { return textureImage; }
		VkImageView getTextureImageView() { return textureImageView; }
		VkSampler getSampler() { return sampler; }
	private:

		void createSampler(VkFilter filterMode , VkSamplerAddressMode overSampleMode, VkBorderColor color, VkBool32 compareAble, VkCompareOp compareOp);
		Device& device;

		VkFormat textureFormat;

		VkSampler sampler;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
	};


}
