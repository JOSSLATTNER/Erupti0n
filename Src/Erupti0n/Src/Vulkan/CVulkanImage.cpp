#include "Vulkan\CVulkanImage.h"

namespace Erupti0n
{
	CVulkanImage::CVulkanImage(CVulkanImageCreateInfo & a_rviCI)
	{
		this->m_pVkPhysicalDevice = a_rviCI.pVkPhysicalDevice;
		this->m_pVkDevice = a_rviCI.pVkDevice;
		this->m_pVKCommandBufferPool = a_rviCI.pVkCommandBufferPool;
		this->m_pTransferQueue = a_rviCI.pVkTransferQueue;

		this->m_UseStaginImage = a_rviCI.useStaginImage;
		this->m_Width = a_rviCI.width;
		this->m_Height = a_rviCI.height;
		this->m_Channels = a_rviCI.channels;

		if (!(this->m_Width*this->m_Height*this->m_Channels))
			this->m_ImageData = (float*)malloc(sizeof(float)*this->m_Width*this->m_Height*this->m_Channels);
		else
			this->m_ImageData = a_rviCI.pData;


		
		this->CreateVulkanImage(this->m_Image, this->m_ImageMemory, a_rviCI);


		if (this->m_UseStaginImage)
		{
			CVulkanImageCreateInfo staginCI = a_rviCI;
			staginCI.imageTilling = VK_IMAGE_TILING_LINEAR;
			staginCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			staginCI.propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

			this->CreateVulkanImage(this->m_StaginImage, this->m_StaginMemory, staginCI);
		}
		
		
		if (a_rviCI.MapTextureDirectly)
		{
			this->MapTexture(a_rviCI);
			
		}

	}


	CVulkanImage::~CVulkanImage()
	{
		free(this->m_ImageData);
	}


	int32_t CVulkanImage::FindMemoryType(VkMemoryRequirements& a_MemRequirements, VkMemoryPropertyFlags a_MemPropertyFlags)
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

	void CVulkanImage::MapTexture(CVulkanImageCreateInfo& a_rviCI)
	{
		VkImage* dstImage = &this->m_Image;
		VkDeviceMemory* dstImageMemory = &this->m_ImageMemory;

		this->AllocImageMemory(this->m_Image, this->m_ImageMemory, a_rviCI.propertyFlags);

		if (this->m_UseStaginImage)
		{
			this->AllocImageMemory(this->m_StaginImage, this->m_StaginMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

			dstImage = &this->m_StaginImage;
			dstImageMemory = &this->m_StaginMemory;
		}

		size_t si = sizeof(int32_t);

		void* dataptr;
		auto resukt = vkMapMemory(*this->m_pVkDevice, *dstImageMemory, 0, this->m_Width*this->m_Height*this->m_Channels*si, 0, &dataptr);
		memcpy(dataptr, this->m_ImageData,this->m_Width*this->m_Height*this->m_Channels*si);
		vkUnmapMemory(*this->m_pVkDevice, *dstImageMemory);


		this->CreateVulkanImageView(a_rviCI);

		//if(this->m_UseStaginImage)
			this->StageImageData();
	}

	VkImageView & const CVulkanImage::GetImageView()
	{
		return this->m_ImageView;
	}

	void CVulkanImage::CreateVulkanImage(VkImage & a_rVkImage, VkDeviceMemory & a_rImageMemory, CVulkanImageCreateInfo & a_rviCI)
	{
		VkImageCreateInfo iCI{};
		iCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		iCI.imageType = VK_IMAGE_TYPE_2D;
		iCI.extent.width = a_rviCI.width;
		iCI.extent.height = a_rviCI.height;
		iCI.extent.depth = 1;
		iCI.mipLevels = 1;
		iCI.arrayLayers = 1;
		iCI.format = a_rviCI.format;
		iCI.tiling = a_rviCI.imageTilling;
		iCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		iCI.usage = a_rviCI.usage;
		iCI.samples = VK_SAMPLE_COUNT_1_BIT;
		iCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto result = vkCreateImage(*a_rviCI.pVkDevice, &iCI, nullptr, &a_rVkImage);
		VkAssert(result, "Failed to create Stagin Image");

	}

	void CVulkanImage::AllocImageMemory(VkImage & a_rVkImage, VkDeviceMemory & a_rVKImageMemory, VkMemoryPropertyFlags a_PropertyFlags)
	{
		VkMemoryRequirements memReq{};
		vkGetImageMemoryRequirements(*this->m_pVkDevice, a_rVkImage, &memReq);

		VkMemoryAllocateInfo maI{};
		maI.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		maI.allocationSize = memReq.size;
		maI.memoryTypeIndex = this->FindMemoryType(memReq, a_PropertyFlags);

		auto result = vkAllocateMemory(*this->m_pVkDevice, &maI, nullptr, &a_rVKImageMemory);
		VkAssert(result, "[Error] Faild to allocate Stating image memory!");

		vkBindImageMemory(*this->m_pVkDevice, a_rVkImage, a_rVKImageMemory, 0);
	}

	void CVulkanImage::CreateVulkanImageView(CVulkanImageCreateInfo & a_rImageCreateInfo)
	{

		VkImageViewCreateInfo ivCI{};
		ivCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivCI.image = this->m_Image;
		ivCI.format = a_rImageCreateInfo.format;
		ivCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivCI.components.r = VK_COMPONENT_SWIZZLE_R;
		ivCI.components.g = VK_COMPONENT_SWIZZLE_G;
		ivCI.components.b = VK_COMPONENT_SWIZZLE_B;
		ivCI.components.a = VK_COMPONENT_SWIZZLE_A;

		ivCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivCI.subresourceRange.layerCount = 1;
		ivCI.subresourceRange.baseArrayLayer = 0;
		ivCI.subresourceRange.levelCount = 1;
		ivCI.subresourceRange.baseMipLevel = 0;

		auto result = vkCreateImageView(*this->m_pVkDevice, &ivCI, nullptr, &this->m_ImageView);
		VkAssert(result, "[ERROR] Can't create VKImageView!");
	}

	void CVulkanImage::StageImageData()
	{
		VkImageMemoryBarrier siB{};
		VkImageMemoryBarrier iB{};
		VkImageMemoryBarrier isB{};

		VkImageSubresourceLayers iSL{};
		VkImageCopy cpyrg{};

		siB.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		siB.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		siB.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		siB.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		siB.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		siB.image = this->m_StaginImage;
		siB.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		siB.subresourceRange.layerCount = 1;
		siB.subresourceRange.levelCount = 1;
		this->GetAccessMasks(siB);

		iB.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		iB.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		iB.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		iB.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		iB.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		iB.image = this->m_Image;
		iB.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		iB.subresourceRange.layerCount = 1;
		iB.subresourceRange.levelCount = 1;
		iB.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		iB.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//this->GetAccessMasks(iB);

		isB.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		isB.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		isB.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		isB.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		isB.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		isB.image = this->m_Image;
		isB.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		isB.subresourceRange.layerCount = 1;
		isB.subresourceRange.levelCount = 1;
		this->GetAccessMasks(isB);
		

		iSL.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		iSL.layerCount = 1;

		cpyrg.srcSubresource = iSL;
		cpyrg.dstSubresource = iSL;
		cpyrg.extent.width = this->m_Width;
		cpyrg.extent.height = this->m_Height;
		cpyrg.extent.depth = 1;

		VkCommandBufferAllocateInfo cbAI{};
		cbAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbAI.commandPool = *this->m_pVKCommandBufferPool;
		cbAI.commandBufferCount = 1;
		cbAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer tmpTransferBuffer;
		vkAllocateCommandBuffers(*this->m_pVkDevice, &cbAI, &tmpTransferBuffer);

		VkCommandBufferBeginInfo cbBI{};
		cbBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cbBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


		{
			vkBeginCommandBuffer(tmpTransferBuffer, &cbBI);

			//vkCmdPipelineBarrier(tmpTransferBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &siB);
			vkCmdPipelineBarrier(tmpTransferBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &iB);
			
			//vkCmdCopyImage(tmpTransferBuffer, this->m_StaginImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpyrg);
			
			//vkCmdPipelineBarrier(tmpTransferBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &isB);
			

			vkEndCommandBuffer(tmpTransferBuffer);
		}
		
		VkPipelineStageFlags watiStage = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo SI{};
		SI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SI.commandBufferCount = 1;
		SI.pCommandBuffers = &tmpTransferBuffer;
		SI.pWaitDstStageMask = &watiStage;

		vkQueueSubmit(*this->m_pTransferQueue, 1, &SI, VK_NULL_HANDLE);
		vkQueueWaitIdle(*this->m_pTransferQueue);

		vkFreeCommandBuffers(*this->m_pVkDevice, *this->m_pVKCommandBufferPool, 1, &tmpTransferBuffer);
	}

	void CVulkanImage::GetAccessMasks(VkImageMemoryBarrier & a_rIMB)
	{
		if (a_rIMB.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && a_rIMB.newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			a_rIMB.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			a_rIMB.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		else if (a_rIMB.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && a_rIMB.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			a_rIMB.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			a_rIMB.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else if (a_rIMB.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && a_rIMB.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			a_rIMB.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			a_rIMB.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}

	}

}