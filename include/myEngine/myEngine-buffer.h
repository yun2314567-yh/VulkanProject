#pragma once
#include"myEngine-device.h"

namespace myEngine
{
	class Buffer
	{
	public:
		Buffer(Device& device, VkDeviceSize instanceSize,uint32_t instanceCount,  VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkDeviceSize minOffsetAlignment);
		

		Buffer(const Buffer&) = delete;
		Buffer& operator =(const Buffer&) = delete;

		~Buffer()
		{
			unMapMemory();
			vkDestroyBuffer(device.getLogicalDevice(), _buffer, nullptr);
			vkFreeMemory(device.getLogicalDevice(), _bufferMemory, nullptr);
		}

		VkBuffer getBuffer() { return _buffer; }	
		
		VkDeviceSize getBufferSize() { return bufferSize; }
		
		VkDescriptorBufferInfo descriptorBufferBuild(VkDeviceSize range=VK_WHOLE_SIZE, VkDeviceSize offset=0);
		

		void mapMemory(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void unMapMemory();
		 
		void copyBuffer(VkBuffer srcBuffer, VkBuffer targetBuffer, VkDeviceSize size);
		
		void copyBuffer2Image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		void writeInBuffer(void *bufferData,VkDeviceSize size=VK_WHOLE_SIZE, VkDeviceSize offset=0);
		


		//使目标内存的更新对GPU可见
		void flushBuffer(VkDeviceSize flushSize, VkDeviceSize offset);
		
		//使目标内存的更新对CPU可见
		void invalidateBuffer(VkDeviceSize invalidateSize, VkDeviceSize offset);

		//按索引写入数据
		void writeWithIndex(void* bufferData, int Index);
		
		void flushBufferWithIndex(int Index);

		void invalidateBufferWithIndex(int Index);

		VkDescriptorBufferInfo indexDescriptorBufferBuild(int index);

	private:
		static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

		Device& device;
		VkDeviceSize instanceSize;
		VkDeviceSize bufferSize;
		VkBufferUsageFlags usage;
		VkMemoryPropertyFlags propertyFlags;
		VkDeviceSize alignmentSize;
		
		void* memoryData=nullptr;
		VkBuffer _buffer=VK_NULL_HANDLE;
		VkDeviceMemory _bufferMemory=VK_NULL_HANDLE;
		
	};

}