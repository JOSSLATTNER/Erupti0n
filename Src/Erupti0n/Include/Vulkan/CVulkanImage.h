#pragma once
#include "Shared.h"
#include "Utilities\TSingleton.hpp"

namespace Erupti0n
{
	struct CVulkanImageCreateInfo
	{
		VkPhysicalDevice* pVkPhysicalDevice;
		VkDevice* pVkDevice;
		VkCommandPool* pVkCommandBufferPool;
		VkQueue* pVkTransferQueue;

		void* pData;
		uint32_t width;
		uint32_t height;
		uint32_t channels;
		VkFormat format;
		VkImageTiling imageTilling;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags  propertyFlags;
		bool MapTextureDirectly;
		bool useStaginImage;
	};

	class CVulkanImage
	{
	private:
		VkPhysicalDevice* m_pVkPhysicalDevice;
		VkDevice* m_pVkDevice;
		VkCommandPool* m_pVKCommandBufferPool;
		VkQueue* m_pTransferQueue;


		bool m_UseStaginImage;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Channels;

		void* m_ImageData;

		VkImage m_Image;
		VkDeviceMemory m_ImageMemory;
		VkImageView m_ImageView;

		VkImage m_StaginImage;
		VkDeviceMemory m_StaginMemory;

	public:
		CVulkanImage(CVulkanImageCreateInfo& a_rviCI);
		~CVulkanImage();

	public:
		int32_t FindMemoryType(VkMemoryRequirements& a_MemRequirements, VkMemoryPropertyFlags a_MemPropertyFlags);
		void MapTexture(CVulkanImageCreateInfo& a_rviCI);

		VkImageView& const GetImageView();

	private:
		void CreateVulkanImage(VkImage& a_rVkImage, VkDeviceMemory& a_rImageMemory, CVulkanImageCreateInfo& a_rImageCreateInfo);
		void AllocImageMemory(VkImage& a_rVkImage, VkDeviceMemory& a_rVKImageMemory, VkMemoryPropertyFlags a_PropertyFlagBits);
		void CreateVulkanImageView(CVulkanImageCreateInfo& a_rImageCreateInfo);
		void StageImageData();
		void GetAccessMasks(VkImageMemoryBarrier& a_rIMB);
	};
}