#include<stdexcept>
#include<array>
#include"myEngine/myEngine-buffer.h"
#include"myEngine/myEngine-Material.h"
#include"stb_image.h"
namespace myEngine
{
	Material::Material(Device& device,const std::vector<myTextureType> textureTypes):
		device{device},textureTypes{textureTypes}
	{
		
		
		
		setTextureSetLayout();
		setUBOSetLayout();
		createWhiteTexture();
		createFallbackPinkTexture();
		createDefaultNormalTexture();

		
		
		shaderMgr = std::make_unique<ShaderManager>(device);
	}

	

	void Material::setTexturePool()
	{
		if (textureTypes.empty()) throw std::runtime_error("???????????);

		textureDescriptorPool = DescriptorPool::Builder(device).setmaxSets(500)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureTypes.size())
			.setFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.build();
	}

	void Material::setTextureSetLayout()
	{
		setTexturePool();

		textureSetLayout = DescriptorSetLayout::Builder(device)
			.addBindings(textureTypes, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.setBindingFlags(textureTypes, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
			.build();


	}

	void Material::setUBOPool()
	{
		uboDescriptorPool = DescriptorPool::Builder(device).setmaxSets(30)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMEBUFFERS_IN_SAMETIME)
			.setFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)
			.build();
	}

	void Material::setUBOSetLayout()
	{
		setUBOPool();

		 uboSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		 
	}

	void Material::updateUBO(std::string modelName,int frameIndex,void* bufferData)
	{
		if (uboBuffers.find(modelName) == uboBuffers.end())
		{
			uboBuffers[modelName].resize(MAX_FRAMEBUFFERS_IN_SAMETIME);
			for (int i = 0; i < uboBuffers[modelName].size(); i++)
			{
				uboBuffers[modelName][i] = std::make_unique<Buffer>(device, sizeof(UBO)
					, 1
					, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
					, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
					, 0);
				uboBuffers[modelName][i]->mapMemory();

			}

		}
		if (uboSets.find(modelName) == uboSets.end())
		{
			uboSets[modelName].resize(MAX_FRAMEBUFFERS_IN_SAMETIME);
			for (int i = 0; i < uboSets[modelName].size(); i++)
			{
				auto bufferInfo = uboBuffers[modelName][i]->descriptorBufferBuild();
				bool success = DescriptorWriter(*uboSetLayout, *uboDescriptorPool)
					.writeBuffer(0, bufferInfo)
					.build(uboSets[modelName][i]);

				if (!success)
					throw std::runtime_error("UBO????????????");
			}

		}
		
		currentFrameIndex = frameIndex;
		
		

		


		uboBuffers[modelName][frameIndex]->writeInBuffer(bufferData);
		
		
	}

	const VkDescriptorSet* Material::getCurrentUBOSet(std::string modelName) 
	{ 
		return &uboSets[modelName][currentFrameIndex];
	}

	void Material::updateTexture(VkDescriptorSet targetSet, myTextureType texType, VkDescriptorImageInfo texture)
	{
		DescriptorWriter writer(*textureSetLayout, *textureDescriptorPool);
		writer.writeImage(texType, texture);
		writer.update(targetSet);
	}

	std::shared_ptr<Texture> Material::getOrLoadTexture(const myTextureType texType, const std::string path)
	{
		if (path.empty())
		{
			if (texType == DIFFUSE)return defaultTextures[DIFFUSE];

			else return defaultTextures[NORMAL];
		}
		

		auto it = textureCache.find(path);
		if (it != textureCache.end())return it->second;

		try
		{
			auto tex = loadTextureFromFile(path);
			textureCache[path] = std::shared_ptr<Texture>(tex.release());
			return textureCache[path];
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to load texture: " << path << ", using pink fallback." << std::endl;
			return fallBackTexture;
		}
	}


	std::unique_ptr<Texture> Material::loadTextureFromFile(const std::string& path)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
			throw std::runtime_error("?????????: " + path);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		Buffer stageBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		stageBuffer.mapMemory();
		stageBuffer.writeInBuffer(pixels);
		stbi_image_free(pixels);

		auto texture = std::make_unique<Texture>(device, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT);


		device.transitionImageLayout(texture->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		device.copyBuffer2Image(stageBuffer.getBuffer(), texture->getTextureImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		device.transitionImageLayout(texture->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		texture->createTextureImageView();
		texture->setSampler();
		return texture;
	}

	void Material::createPipelineLayout(std::deque<VkDescriptorSetLayout> SetLayout)
	{
		
		
		SetLayout.push_front(uboSetLayout->getDescriptorSetLayout());
		SetLayout.push_front(textureSetLayout->getDescriptorSetLayout());
		std::vector<VkDescriptorSetLayout> layouts{SetLayout.begin(), SetLayout.end()};

		
	
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(SetLayout.size());
	
		pipelineLayoutInfo.pSetLayouts = layouts.data();

	
		// If a pipeline layout already exists for this Material, destroy and unregister it
	
		if (pipelineLayout != VK_NULL_HANDLE) {
		
			vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);		
			pipelineLayout = VK_NULL_HANDLE;
	
		}
	
		if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("????????????");

	
		// Register created pipeline layout with Device for tracking	
		}

	void  Material::createWhiteTexture()
	{
		int texWidth = 1, texHeight = 1;

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		unsigned char pixels[] = { 255, 255, 255, 255 }; // ?????????

		Buffer stageBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		stageBuffer.mapMemory();
		stageBuffer.writeInBuffer(pixels);


		auto whiteTexture = std::make_shared<Texture>(device, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT);

		device.transitionImageLayout(whiteTexture->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		device.copyBuffer2Image(stageBuffer.getBuffer(), whiteTexture->getTextureImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		device.transitionImageLayout(whiteTexture->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		whiteTexture->createTextureImageView();
		whiteTexture->setSampler();

		defaultTextures[DIFFUSE] = whiteTexture;
		defaultTextures[SPECULAR] = whiteTexture;
	}

	void Material::createDefaultNormalTexture()
	{
		unsigned char pixels[] = { 0,0,255,1 };
		int texWidth = 1, texHeight = 1;

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		Buffer stageBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		stageBuffer.mapMemory();
		stageBuffer.writeInBuffer(pixels);


		auto normalMap = std::make_shared<Texture>(device, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT);

		device.transitionImageLayout(normalMap->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		device.copyBuffer2Image(stageBuffer.getBuffer(), normalMap->getTextureImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		device.transitionImageLayout(normalMap->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		normalMap->createTextureImageView();
		normalMap->setSampler();

		defaultTextures[NORMAL] = normalMap;
	}

	void Material::createFallbackPinkTexture()
	{
		unsigned char pixels[] = { 255,179,183,1 };
		int texWidth = 1, texHeight = 1;

		VkDeviceSize imageSize = texWidth * texHeight * 4;

		Buffer stageBuffer(device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
		stageBuffer.mapMemory();
		stageBuffer.writeInBuffer(pixels);


		fallBackTexture = std::make_shared<Texture>(device, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_SAMPLE_COUNT_1_BIT);

		device.transitionImageLayout(fallBackTexture->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		device.copyBuffer2Image(stageBuffer.getBuffer(), fallBackTexture->getTextureImage(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		device.transitionImageLayout(fallBackTexture->getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		fallBackTexture->createTextureImageView();
		fallBackTexture->setSampler();

		
	}

	
}