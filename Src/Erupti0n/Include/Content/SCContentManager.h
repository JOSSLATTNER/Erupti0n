#pragma once

#include <map>

#include "Shared.h"
#include "Content\CFileloader.h"

#include "Vulkan\CVulkanImage.h"

namespace Erupti0n
{

	struct SLoadTextureInfo
	{
		VkDevice* pVkDevice;
		VkPhysicalDevice* pVkPhysicalDevice;
		VkCommandPool* pVkCommandBufferPool;
		VkQueue* pVkTransferQueue;

		std::string a_rFilePath;
		uint32_t a_rWidth;
		uint32_t a_rHeight;
		uint32_t a_rChannels;
	};

	class SCContentManager
	{
	private:
		CFileloader m_FileLoader;
		std::map<std::string, CVulkanImage*> m_VulkanImages;

	public:
		SCContentManager();
		~SCContentManager();

	public:
		std::vector<char> LoadBinaryFile(std::string& const a_rFilePath);
		CVulkanImage& LoadTexture(SLoadTextureInfo& a_rltI);
	};
}