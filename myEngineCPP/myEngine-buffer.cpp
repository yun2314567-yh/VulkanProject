#include"myEngine/myEngine-buffer.h"
#include<cassert>
namespace  myEngine
{
	Buffer::Buffer(Device& device, VkDeviceSize instanceSize,uint32_t instanceCount, VkBufferUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkDeviceSize minOffsetAlignment)
		: device{ device }, instanceSize{ instanceSize }, usage{ usage }, propertyFlags{ propertyFlags }
	{
		alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
		bufferSize = alignmentSize * instanceCount;//开辟空间
		device.createBuffer(usage, propertyFlags, bufferSize, _buffer, _bufferMemory);
	}

	

	void Buffer::mapMemory(VkDeviceSize size, VkDeviceSize offset)
	{
		
		vkMapMemory(device.getLogicalDevice(), _bufferMemory, offset, size, 0, &memoryData);
		
	}

	

	void Buffer::writeInBuffer(void* bufferData,VkDeviceSize size,VkDeviceSize offset)
	{
		assert(memoryData && "未分配内存时就使用内存复制");

		//size为使用空间范围
		if(size==VK_WHOLE_SIZE)
		memcpy(memoryData, bufferData, bufferSize);

		else
		{
			char* memoryDataIsOffset = static_cast<char*>(memoryData) + offset;

			memcpy(memoryDataIsOffset, bufferData, size);
		}

		
	}

	void Buffer::flushBuffer(VkDeviceSize flushSize,VkDeviceSize offset)
	{
		//刷新开辟空间，使目标内存的更新对GPU可见
		VkMappedMemoryRange memoryRange;
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = _bufferMemory;
		memoryRange.size = flushSize;
		memoryRange.offset = offset;
		vkFlushMappedMemoryRanges(device.getLogicalDevice(), 1, &memoryRange);
	}

	void Buffer::invalidateBuffer(VkDeviceSize invalidateSize, VkDeviceSize offset)
	{
		//刷新开辟空间，使目标内存的更新对CPU可见
		VkMappedMemoryRange memoryRange;
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = _bufferMemory;
		memoryRange.size = invalidateSize;
		memoryRange.offset = offset;
		vkInvalidateMappedMemoryRanges(device.getLogicalDevice(), 1, &memoryRange);
	}


	VkDescriptorBufferInfo Buffer::descriptorBufferBuild(VkDeviceSize range, VkDeviceSize offset)
	{
		return { _buffer,offset,range };
	}

	

	void Buffer::writeWithIndex(void* bufferData, int Index)
	{
		writeInBuffer(bufferData, instanceSize, Index * alignmentSize);
	}


	void Buffer::flushBufferWithIndex(int Index)
	{
		flushBuffer(alignmentSize, Index * alignmentSize);
	}

	void Buffer::invalidateBufferWithIndex(int Index)
	{
		invalidateBuffer(alignmentSize, Index * alignmentSize);
	}
	
	VkDescriptorBufferInfo Buffer::indexDescriptorBufferBuild(int index)
	{
		return descriptorBufferBuild(alignmentSize, index * alignmentSize);
	}

	void Buffer::unMapMemory()
	{

		if (memoryData)//防悬空指针
		{
			vkUnmapMemory(device.getLogicalDevice(), _bufferMemory);
			memoryData = nullptr;
		}
	}

	VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0) {
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}
		
	    return instanceSize;
		
	}

}