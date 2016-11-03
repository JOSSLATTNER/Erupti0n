#include "Vulkan\CVulkanBufferBase.h"

namespace Erupti0n
{
	CVulkanBufferBase::CVulkanBufferBase(CVulkanBufferBaseCreateInfo & a_rvbbCI)
	{
		this->m_pVkPhysicalDevice = a_rvbbCI.pVkPhysicalDevice;
		this->m_pVkDevice = a_rvbbCI.pVkDevice;
		this->m_pVkCommandpool = a_rvbbCI.pCommandBufferpool;
		this->m_pQueue = a_rvbbCI.pQueue;

		this->m_bufferSize = a_rvbbCI.size;
		this->m_pData = a_rvbbCI.pData;
		this->m_useStageBuffer = a_rvbbCI.useStagingBuffer;

		VkBufferCreateInfo bCI{};
		bCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bCI.size = this->m_bufferSize;
		bCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | a_rvbbCI.usage;
		bCI.sharingMode = a_rvbbCI.sharingMode;

		auto result = vkCreateBuffer(*this->m_pVkDevice, &bCI, nullptr, &this->m_BufferHandle);
		VkAssert(result, "Failed to create Buffer");
		

		VkMemoryRequirements bmr, sbmr;
		vkGetBufferMemoryRequirements(*this->m_pVkDevice, this->m_BufferHandle, &bmr);
		VkMemoryAllocateInfo mai{};
		mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mai.allocationSize = bmr.size;
		mai.memoryTypeIndex = FindMemoryType(bmr,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		result = vkAllocateMemory(*this->m_pVkDevice, &mai, nullptr, &this->m_BufferMemory);
		VkAssert(result, "[ERROR] Failed to allocate vkVertexBuffer!");
		vkBindBufferMemory(*this->m_pVkDevice, this->m_BufferHandle, this->m_BufferMemory, 0);

		if (this->m_useStageBuffer)
		{
			bCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bCI.size = this->m_bufferSize;
			bCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bCI.sharingMode = a_rvbbCI.sharingMode;


			result = vkCreateBuffer(*this->m_pVkDevice, &bCI, nullptr, &this->m_StagingBufferHandle);
			VkAssert(result, "Failed to create mBuffer");

			VkMemoryRequirements sbmr;
			vkGetBufferMemoryRequirements(*this->m_pVkDevice, this->m_StagingBufferHandle, &sbmr);

			VkMemoryAllocateInfo mai{};
			mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			mai.allocationSize = sbmr.size;
			mai.memoryTypeIndex = FindMemoryType(sbmr,VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			result = vkAllocateMemory(*this->m_pVkDevice, &mai, nullptr, &this->m_StagingMemory);
			VkAssert(result, "[ERROR] Failed to allocate vkVertexBuffer!");

			vkBindBufferMemory(*this->m_pVkDevice, this->m_StagingBufferHandle, this->m_StagingMemory, 0);

			
		}
		if(this->m_bufferSize > 0)
		Updatebuffer();
	}

	CVulkanBufferBase::~CVulkanBufferBase()
	{
		vkFreeMemory(*this->m_pVkDevice, this->m_BufferMemory, nullptr);
		vkFreeMemory(*this->m_pVkDevice, this->m_StagingMemory, nullptr);
	}

	void CVulkanBufferBase::Updatebuffer()
	{
		if (this->m_useStageBuffer)
		{
			this->StageMemoryData();
		}
		else
		{
			this->FlushMemoryData();
		}
	}

	void CVulkanBufferBase::UpdateBuffer(size_t a_Offset, size_t a_Length)
	{
		
	}

	VkBuffer& CVulkanBufferBase::GetBufferHandle()
	{
		return this->m_BufferHandle;
	}

	VkDeviceMemory& CVulkanBufferBase::GetBufferMemory()
	{
		return this->m_BufferMemory;
	}

	size_t CVulkanBufferBase::GetBufferSize()
	{
		return this->m_bufferSize;
	}

	int32_t CVulkanBufferBase::FindMemoryType(VkMemoryRequirements& a_MemRequirements, VkMemoryPropertyFlagBits a_MemPropertyFlags)
	{
		VkPhysicalDeviceMemoryProperties pdmp{};
		vkGetPhysicalDeviceMemoryProperties(*this->m_pVkPhysicalDevice, &pdmp);

		for (int32_t i = 0; i < pdmp.memoryTypeCount; ++i)
		{
			if ((a_MemRequirements.memoryTypeBits & (1 << i)) && (pdmp.memoryTypes[i].propertyFlags & a_MemPropertyFlags))
			{
				return i;
			}
		}
	}

	void CVulkanBufferBase::FlushMemoryData()
	{
		VkMappedMemoryRange fr{};
		fr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		fr.memory = this->m_BufferMemory;
		fr.offset = 0;
		fr.size = VK_WHOLE_SIZE;

		void* vbmp;
		auto result = vkMapMemory(*this->m_pVkDevice, this->m_BufferMemory, 0, this->m_bufferSize, 0, &vbmp);
		VkAssert(result, "[ERROR] Failed to map data to buffer!");
		memcpy(vbmp, this->m_pData, this->m_bufferSize);
		vkFlushMappedMemoryRanges(*this->m_pVkDevice, 1, &fr);
		vkUnmapMemory(*this->m_pVkDevice, this->m_BufferMemory);
	}

	void CVulkanBufferBase::StageMemoryData()
	{
		VkCommandBufferAllocateInfo cbAI{};
		cbAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbAI.commandPool = *this->m_pVkCommandpool;
		cbAI.commandBufferCount = 1;
		cbAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer tmpTransferBuffer;
		vkAllocateCommandBuffers(*this->m_pVkDevice, &cbAI, &tmpTransferBuffer);

		void* vbmp;
		auto result = vkMapMemory(*this->m_pVkDevice, this->m_StagingMemory, 0, this->m_bufferSize, 0, &vbmp);
		VkAssert(result, "[ERROR] Failed to map data to buffer!");
		memcpy(vbmp, this->m_pData, this->m_bufferSize);
		vkUnmapMemory(*this->m_pVkDevice, this->m_StagingMemory);

		VkCommandBufferBeginInfo cbBI{};
		cbBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cbBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


		vkBeginCommandBuffer(tmpTransferBuffer, &cbBI);

		VkBufferCopy cb{};
		cb.size = this->m_bufferSize;
		vkCmdCopyBuffer(tmpTransferBuffer, this->m_StagingBufferHandle, this->m_BufferHandle, 1, &cb);
		vkEndCommandBuffer(tmpTransferBuffer);

		VkSubmitInfo SI{};
		SI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SI.commandBufferCount = 1;
		SI.pCommandBuffers = &tmpTransferBuffer;

		vkQueueSubmit(*this->m_pQueue, 1, &SI, VK_NULL_HANDLE);
		vkQueueWaitIdle(*this->m_pQueue);

		vkFreeCommandBuffers(*this->m_pVkDevice, *this->m_pVkCommandpool, 1, &tmpTransferBuffer);
	}
}