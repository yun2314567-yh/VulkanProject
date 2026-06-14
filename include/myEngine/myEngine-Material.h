#pragma once
#include<unordered_map>
#include<memory>
#include<vector>
#include"myEngine/myEngine-device.h"
#include"myEngine/myEngine-Descriptor.h"
#include"myEngine/myEngine-buffer.h"
#include"myEngine/myEngine-Texture.h"
#include"myEngine/myEngine-ShaderManager.h"
namespace myEngine
{
	class Material
	{
	public:
		Material(Device& device, const std::vector<myTextureType> textureTypes);

		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;

		~Material()
		{
			if (pipelineLayout != VK_NULL_HANDLE)
			{
				vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
				pipelineLayout = VK_NULL_HANDLE;
			}
		}

		std::string vertShaderPath = "compiledShaders/blankShader_vert.spv";
		std::string fragShaderPath = "compiledShaders/blankShader_frag.spv";

		VkDescriptorSetLayout getTextureSetLayout() { return textureSetLayout->getDescriptorSetLayout(); }
		VkDescriptorSetLayout getUBOSetLayout() { return uboSetLayout->getDescriptorSetLayout(); }
		void setTextureSetLayout();

		void updateTexture(VkDescriptorSet targetSet, myTextureType texType, VkDescriptorImageInfo texture);
		
		void updateUBO(std::string modelName, int frameIndex, void* bufferData);

		void allocatePool(VkDescriptorSet& targetDesSet)
		{
			if (!textureDescriptorPool->allocateDescriptor(textureSetLayout->getDescriptorSetLayout(), targetDesSet))
				throw std::runtime_error("分配描述符池失败");
			if(targetDesSet==VK_NULL_HANDLE)throw std::runtime_error("cj失败");
		}

		std::unique_ptr<ShaderManager>& getShaderManager() { return shaderMgr; }

		const std::vector<myTextureType>& getTextureTypes() { return textureTypes; }

		std::shared_ptr<Texture> getDefaultTexture(myTextureType texType) { return defaultTextures[texType]; }
	
		const VkDescriptorSet* getCurrentUBOSet(std::string modelName); 
		
		std::shared_ptr<Texture> getOrLoadTexture(const myTextureType texType, const std::string path);

		void createPipelineLayout(std::deque<VkDescriptorSetLayout> SetLayout);

		VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

	private:
		int currentFrameIndex = 0;

		std::unique_ptr<Texture> loadTextureFromFile(const std::string& path);
		void createFallbackPinkTexture();
		void createWhiteTexture();
		void createDefaultNormalTexture();
		void setTexturePool();
		void setUBOPool();
		
		void setUBOSetLayout();


		Device& device;

		std::unordered_map<std::string,std::vector<std::unique_ptr<Buffer>>> uboBuffers;

		std::unique_ptr<DescriptorPool> uboDescriptorPool;
		std::unique_ptr<DescriptorPool> textureDescriptorPool;
		
		std::unique_ptr<DescriptorSetLayout> uboSetLayout;
		std::unique_ptr<DescriptorSetLayout> textureSetLayout;
		

		std::unordered_map<std::string ,std::vector<VkDescriptorSet>> uboSets;

		std::vector<Texture> loadTexture;


		std::unordered_map<myTextureType, std::shared_ptr<Texture>> defaultTextures;
		//std::shared_ptr<Texture> whiteTexture;
		//std::shared_ptr<Texture> normalMap;
		std::shared_ptr<Texture> fallBackTexture;


		VkPipelineLayout pipelineLayout=VK_NULL_HANDLE;

		std::vector<myTextureType> textureTypes;

		std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;

		//DescriptorWriter writer;

		std::unique_ptr<ShaderManager> shaderMgr;


		
	};
}