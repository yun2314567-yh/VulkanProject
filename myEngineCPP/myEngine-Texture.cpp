#include"myEngine/myEngine-Texture.h"

namespace myEngine
{
	Texture::Texture(Device& device, const uint32_t& texWidth, const uint32_t& texHeight, const VkImageType& imageType, const VkFormat& format,
		const VkImageTiling& tiling, const VkImageUsageFlags& usage, const VkSharingMode& sharingMode,
		const VkSampleCountFlagBits& sampleWay) :device{ device }
	{
		device.createImage(texWidth, texHeight, imageType, format, tiling, usage, sharingMode, sampleWay, textureImage, textureImageMemory);
	 }

	VkDescriptorImageInfo Texture::descriptorImageInfoBuild(VkImageLayout imageLayout )
	{
		return { sampler, textureImageView, imageLayout};
	}

	void Texture::createSampler(VkFilter filterMode,VkSamplerAddressMode overSampleMode,VkBool32 isShadowMap,VkCompareOp shadowDepthCompareMode)
	{
		
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.minFilter = filterMode;
		samplerInfo.magFilter = filterMode;
		samplerInfo.addressModeU = overSampleMode;
		samplerInfo.addressModeV = overSampleMode;
		samplerInfo.addressModeW = overSampleMode;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 8;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = isShadowMap;
		samplerInfo.compareOp = shadowDepthCompareMode;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0;

		samplerInfo.maxLod = 0;
		samplerInfo.minLod = 0;

		if (vkCreateSampler(device.getLogicalDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			throw std::runtime_error("采样器创建失败");
	}
}