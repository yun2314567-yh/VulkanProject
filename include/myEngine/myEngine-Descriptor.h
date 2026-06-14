#pragma once

#include<memory>
#include<vector>
#include<unordered_map>

#include"myEngine-device.h"
namespace myEngine {

	

	class DescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder(Device& device) : device{ device } {}
			Builder& addBinding(uint32_t bindingLocation, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count = 1);
			Builder& addBindings(std::vector<myTextureType> textureTypes, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count = 1);
			Builder& setBindingFlags(std::vector<myTextureType> textureTypes, VkDescriptorBindingFlags flags);
			std::unique_ptr<DescriptorSetLayout> build() const;
			
		private:
			Device& device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindingsInfo;
			std::unordered_map<uint32_t, VkDescriptorBindingFlags> bindingFlags;
		};

		DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindingsInfo, std::unordered_map<uint32_t, VkDescriptorBindingFlags> bindingFlags);
		
		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

		std::vector<myTextureType> getBindingType();
		
		~DescriptorSetLayout()
		{
			vkDestroyDescriptorSetLayout(device.getLogicalDevice(), descriptorSetLayout, nullptr);
		}

		

	private:
		
		
		VkDescriptorSetLayout descriptorSetLayout;
		
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindingsInfo;
		std::unordered_map<uint32_t, VkDescriptorBindingFlags> bindingFlags;

		Device& device;
		
		friend class DescriptorWriter;

	};
	
	class DescriptorPool
	{
	public:
		class Builder
		{
			public:
			Builder(Device& device) : device{ device } {}
			Builder& addPoolSize(VkDescriptorType descriptorType,uint32_t count);
			Builder& setmaxSets(uint32_t maxSets) { this->maxSets = maxSets; return *this; }
			Builder& setFlags(VkDescriptorPoolCreateFlags flags) { this->flags = flags; return *this; }
			std::unique_ptr<DescriptorPool> build() const;

		private:
			Device& device;
			VkDescriptorPoolCreateFlags flags = 0;
			std::vector<VkDescriptorPoolSize> poolSizes;
			uint32_t maxSets=1000;
		};

		DescriptorPool(Device& device,uint32_t maxSets, VkDescriptorPoolCreateFlags flags,const std::vector<VkDescriptorPoolSize> &poolSizes);

		~DescriptorPool()
		{
			vkDestroyDescriptorPool(device.getLogicalDevice(), descriptorPool, nullptr);
		}



		bool allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptorSets);
		

		void resetPool();
		

	private:
		Device& device;
		std::vector<VkDescriptorPoolSize> poolSizes;
		VkDescriptorPool descriptorPool;
	    
		friend class DescriptorWriter;
	};
	
	
	class DescriptorWriter
	{
	public:
		DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);
		DescriptorWriter& writeBuffer(uint32_t bindingLocation, VkDescriptorBufferInfo* bufferInfo);
		DescriptorWriter& writeImage(uint32_t bindingLocation, VkDescriptorImageInfo* imageInfo);
		bool build(VkDescriptorSet& descriptorSet);//在指令写入时直接调用
		void update(VkDescriptorSet& descriptorSet);
	private:
		void overwrite(VkDescriptorSet& descriptorSet);
		DescriptorSetLayout& setLayout;
		DescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> writes;
		 
	};
	
}