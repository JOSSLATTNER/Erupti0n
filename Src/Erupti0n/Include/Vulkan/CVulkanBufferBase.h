#pragma once
#include "Shared.h"

namespace Erupti0n
{
	struct CVulkanBufferBaseCreateInfo
	{
		VkPhysicalDevice* pVkPhysicalDevice;
		VkDevice* pVkDevice;
		VkCommandPool* pCommandBufferpool;
		VkQueue* pQueue;

		VkDeviceSize size;
		void* pData;
		VkBufferUsageFlags usage;
		VkSharingMode sharingMode;
		bool useStagingBuffer;
	};


	class CVulkanBufferBase
	{
	protected:
		VkPhysicalDevice* m_pVkPhysicalDevice;
		VkDevice* m_pVkDevice;
		VkCommandPool* m_pVkCommandpool;
		VkQueue* m_pQueue;

		VkBuffer m_BufferHandle;
		VkDeviceMemory m_BufferMemory;

		VkBuffer m_StagingBufferHandle;
		VkDeviceMemory m_StagingMemory;

		void* m_pData;
		uint32_t m_bufferSize;
		bool m_useStageBuffer;

	public:
		CVulkanBufferBase(CVulkanBufferBaseCreateInfo& a_rvbbCI);
		~CVulkanBufferBase();

	public:
		void Updatebuffer();
		void UpdateBuffer(size_t a_offset, size_t a_length);
		

		VkBuffer& GetBufferHandle();
		VkDeviceMemory& GetBufferMemory();
		size_t GetBufferSize();

	private:
		int32_t FindMemoryType(VkMemoryRequirements& a_MemRequirements, VkMemoryPropertyFlagBits a_MemPropertyFlags);

		void FlushMemoryData();
		void StageMemoryData();
	};
}