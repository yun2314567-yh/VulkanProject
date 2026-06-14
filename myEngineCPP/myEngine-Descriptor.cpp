#include <cassert>
#include <stdexcept>
#include"myEngine/myEngine-Descriptor.h"
namespace myEngine
{
	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(uint32_t bindingLocation, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count)
	{
		VkDescriptorSetLayoutBinding bindingInfo = {};
		bindingInfo.binding = bindingLocation;
		bindingInfo.descriptorType = type;
		bindingInfo.descriptorCount = count;
		bindingInfo.stageFlags = stageFlags;
		
		bindingsInfo[bindingLocation] = bindingInfo;
		return *this;
	}

	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBindings(std::vector<myTextureType> textureTypes, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count)
	{
		for (auto texType : textureTypes)
		{
			VkDescriptorSetLayoutBinding bindingInfo = {};
			bindingInfo.binding = texType;
			bindingInfo.descriptorType = type;
			bindingInfo.descriptorCount = count;
			bindingInfo.stageFlags = stageFlags;

			bindingsInfo[texType] = bindingInfo;
		 }

		return *this;
	}

	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::setBindingFlags(std::vector<myTextureType> textureTypes, VkDescriptorBindingFlags flags)
	{
		for (auto type : textureTypes)
		{
			bindingFlags[type] = flags;
		}
		return *this;
	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<DescriptorSetLayout>(device, bindingsInfo, bindingFlags);

		
		
	}



	DescriptorSetLayout::DescriptorSetLayout(Device & device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindingsInfo,std::unordered_map<uint32_t, VkDescriptorBindingFlags> bindingFlags) : device{device}, bindingsInfo{bindingsInfo}
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		
		for(auto& i: bindingsInfo)
		{
			bindings.push_back(i.second);
		}

		std::vector<VkDescriptorBindingFlags> flagsList;
		flagsList.reserve(bindingsInfo.size());
		for (auto& p : bindingsInfo) {
			uint32_t binding = p.first;
			auto it = bindingFlags.find(binding);
			VkDescriptorBindingFlags flag = (it != bindingFlags.end()) ? it->second : 0;
			flagsList.push_back(flag);
		}


		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t> (bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flagsInfo = {};
		flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		flagsInfo.bindingCount= static_cast<uint32_t>(flagsList.size());
		flagsInfo.pBindingFlags = flagsList.data();

		layoutInfo.pNext = &flagsInfo;

		if (vkCreateDescriptorSetLayout(device.getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("描述符集布局创建失败");
	}

	std::vector<myTextureType> DescriptorSetLayout::getBindingType()
	{
		std::vector<myTextureType> bindingTypes;
		for (auto& i : bindingsInfo)
		{
			bindingTypes.push_back(static_cast<myTextureType>(i.first));
		}
		return bindingTypes;
	}

	
	DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
	{
		 poolSizes.push_back({descriptorType,count});
		return *this;
	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
	{
		return std::make_unique<DescriptorPool>(device, maxSets,flags,poolSizes);
	}

	DescriptorPool::DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags flags, const std::vector<VkDescriptorPoolSize>& poolSizes) : device{ device },  poolSizes{ poolSizes }
	{
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;
		poolInfo.flags = flags;

		if (vkCreateDescriptorPool(device.getLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("描述符池创建失败");

	}

	bool DescriptorPool::allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;
	    
		if (vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
				return false;
			

		return true;

	}

	void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptorSets)
	{
		vkFreeDescriptorSets(device.getLogicalDevice(), descriptorPool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
	}
	
	void DescriptorPool::resetPool()
	{
		vkResetDescriptorPool(device.getLogicalDevice(), descriptorPool, 0);
	}

	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool) 
		: setLayout{ setLayout }, pool{ pool }
	{

	}

	DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t bindingLocation, VkDescriptorBufferInfo* bufferInfo)
	{
		assert(setLayout.bindingsInfo.count(bindingLocation) == 1 && "绑定位置无效");

		auto& bindingsInfo = setLayout.bindingsInfo[bindingLocation];

		

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = bindingLocation;
		write.descriptorType = bindingsInfo.descriptorType;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = bindingsInfo.descriptorCount;
		
		writes.push_back(write);

		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t bindingLocation, VkDescriptorImageInfo* imageInfo)
	{
		assert(setLayout.bindingsInfo.count(bindingLocation) == 1 && "绑定位置无效");
		
		auto& bindingsInfo = setLayout.bindingsInfo[bindingLocation];

		assert(
			bindingsInfo.descriptorCount == 1 &&
			"描述符数量与绑定数量不匹配");

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = bindingLocation;
		write.descriptorType = bindingsInfo.descriptorType;
		write.pImageInfo = imageInfo;
		write.descriptorCount = bindingsInfo.descriptorCount;

		writes.push_back(write);

		return *this;
	}
	
	void DescriptorWriter::update(VkDescriptorSet& descriptorSet)
	{
		if (descriptorSet == VK_NULL_HANDLE)
			throw std::runtime_error("描述符集未赋值/描述符未分配内存");

		overwrite(descriptorSet);
	}

	bool DescriptorWriter::build(VkDescriptorSet& descriptorSet)
	{
		bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), descriptorSet);
		if (!success)
			return false;
		overwrite(descriptorSet);
		return true;
	}

	void DescriptorWriter::overwrite(VkDescriptorSet& descriptorSet)//VkDescriptorSet本身即为一个数组
	{
		for (auto& write : writes)
			write.dstSet = descriptorSet;
		vkUpdateDescriptorSets(pool.device.getLogicalDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}

	
}