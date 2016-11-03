#include "Content\SCContentManager.h"

namespace Erupti0n
{
	SCContentManager::SCContentManager()
	{
	}

	SCContentManager::~SCContentManager()
	{
	}

	std::vector<char> SCContentManager::LoadBinaryFile(std::string& const a_rFilePath)
	{
		return this->m_FileLoader.GetBinaryFile(a_rFilePath);
	}

	CVulkanImage& SCContentManager::LoadTexture(SLoadTextureInfo& a_rltI)
	{
		if (this->m_VulkanImages.count(a_rltI.a_rFilePath))
		{
			LOG("[MESSAGE] Chach HIT");
			return *this->m_VulkanImages[a_rltI.a_rFilePath];
		}

		LOG("[MESSAGE] Chach Miss");
		std::vector<float> imageData = this->m_FileLoader.GetTexture(a_rltI.a_rFilePath, a_rltI.a_rWidth, a_rltI.a_rHeight, a_rltI.a_rChannels);
		CVulkanImageCreateInfo iCI{};
		iCI.pVkDevice = a_rltI.pVkDevice;
		iCI.pVkPhysicalDevice = a_rltI.pVkPhysicalDevice;
		iCI.pVkCommandBufferPool = a_rltI.pVkCommandBufferPool;
		iCI.pVkTransferQueue = a_rltI.pVkTransferQueue;

		//auto data = std::vector<float>{
		//	 255 / 255.0f,0,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f


		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f


		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f

		//	,255 / 255.0f,0.,0
		//	,0,255 / 255.0f,0
		//	,0,0,255 / 255.0f
		//};

		void* da = (void*) new float[3 * 4 * 3];

		for (int i = 0; i < 4; ++i)
		{
			for (int n = 0; n < 3; ++n)
			{
				float* p = ((float*)da) + (n * 4 * 3) + (i * 3);

				p[0] = 255.0f / 255.0f;
				p[1] = 0.0f;
				p[2] = 0.0f;
			}
		}
		

		iCI.channels =  a_rltI.a_rChannels;
		iCI.width =  a_rltI.a_rWidth; //;a_rltI.a_rWidth;
		iCI.height =  a_rltI.a_rHeight; //a_rltI.a_rHeight;
		iCI.format = VK_FORMAT_R32G32B32_UINT;
		iCI.usage =  VK_IMAGE_USAGE_SAMPLED_BIT;
		iCI.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		iCI.imageTilling = VK_IMAGE_TILING_LINEAR;
		
		iCI.pData = (void*)&imageData[0];
		iCI.useStaginImage = false;
		iCI.MapTextureDirectly = true;

		CVulkanImage* tmpImage = new CVulkanImage(iCI);
		this->m_VulkanImages[a_rltI.a_rFilePath] = tmpImage;


		return *tmpImage;
	}
}
