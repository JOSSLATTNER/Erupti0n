#include "Vulkan\CVulkanSwapChain.h"

namespace Erupti0n
{
	CVulkanSwapChain::CVulkanSwapChain(VulkanSwapChainCreationInfo& a_rVulkanSwapChainCreationInfo)
	{
		this->m_pVkInstace = a_rVulkanSwapChainCreationInfo.pVkInstace;
		this->m_pVkPhysicalDevice = a_rVulkanSwapChainCreationInfo.pVkPhysicalDevice;
		this->m_pVkDevice = a_rVulkanSwapChainCreationInfo.pVkDevice;
		this->m_pSurefaceKHR = a_rVulkanSwapChainCreationInfo.pVkSurfaceKHR;
		this->m_pVkDevice = a_rVulkanSwapChainCreationInfo.pVkDevice;	

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*this->m_pVkPhysicalDevice, *this->m_pSurefaceKHR, &surfaceCapabilities);
		if (result != VK_SUCCESS)
		{
			LOG("ERROR: Get Surface capabilities failed!");
		}

		uint32_t numSwapchainImages = surfaceCapabilities.minImageCount + 1;
		if ((surfaceCapabilities.maxImageCount > 0) &&
			(numSwapchainImages > surfaceCapabilities.maxImageCount)) {
			numSwapchainImages = surfaceCapabilities.maxImageCount;
		}

		uint32_t numFormats;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(*this->m_pVkPhysicalDevice, *this->m_pSurefaceKHR, &numFormats, nullptr);
		if (result != VK_SUCCESS)
		{
			LOG("ERROR: Get SurfaceFormats failed!");
		}

		std::vector<VkSurfaceFormatKHR> surfaceFormats(numFormats);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(*this->m_pVkPhysicalDevice, *this->m_pSurefaceKHR, &numFormats, &surfaceFormats[0]);

//		vkGetPhysicalDeviceSurfaceSupportKHR(*this->m_pVkPhysicalDevice,0,)
		// Detect wich is a presnet Format on the PhysicalDevice
		// Check the best suited format
		if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			this->m_VkSurfaceFormatKHR = { VK_FORMAT_R8G8B8A8_UNORM,VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		}
		else
		this->m_VkSurfaceFormatKHR = surfaceFormats[0];

		for (VkSurfaceFormatKHR& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
				this->m_VkSurfaceFormatKHR = surfaceFormat;
		}

		
		this->m_SwapChainExtent = surfaceCapabilities.currentExtent;
		if (surfaceCapabilities.currentExtent.width == -1)
		{
			this->m_SwapChainExtent = { a_rVulkanSwapChainCreationInfo.width, a_rVulkanSwapChainCreationInfo.height };
			
			if (this->m_SwapChainExtent.width < surfaceCapabilities.minImageExtent.width)
				this->m_SwapChainExtent.width == surfaceCapabilities.minImageExtent.width;
			if (this->m_SwapChainExtent.height < surfaceCapabilities.minImageExtent.height)
				this->m_SwapChainExtent.height == surfaceCapabilities.minImageExtent.height;

			if (this->m_SwapChainExtent.width < surfaceCapabilities.maxImageExtent.width)
				this->m_SwapChainExtent.width == surfaceCapabilities.maxImageExtent.width;
			if (this->m_SwapChainExtent.height < surfaceCapabilities.maxImageExtent.height)
				this->m_SwapChainExtent.height == surfaceCapabilities.maxImageExtent.height;
		}

		VkImageUsageFlags iUsageFlags = surfaceCapabilities.supportedUsageFlags;
		if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			iUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VkSurfaceTransformFlagBitsKHR swapChainTransform = surfaceCapabilities.currentTransform;
		if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			swapChainTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;


		uint32_t numPresentModes;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(*this->m_pVkPhysicalDevice, *this->m_pSurefaceKHR, &numPresentModes, nullptr);
		if (result != VK_SUCCESS)
		{
			LOG("[ERROR] Get numPresentModes failed!");
		}

		VkPresentModeKHR* pPresentModes = new VkPresentModeKHR[numPresentModes];
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(*this->m_pVkPhysicalDevice, *this->m_pSurefaceKHR, &numPresentModes, pPresentModes);


		VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (int i = 0; i < numPresentModes; ++i)
		{
			if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}

		VkSwapchainCreateInfoKHR scCI{};
		scCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		scCI.surface = *this->m_pSurefaceKHR;
		scCI.minImageCount = numSwapchainImages;
		scCI.imageFormat = this->m_VkSurfaceFormatKHR.format;
		scCI.imageExtent = this->m_SwapChainExtent;
		scCI.preTransform = swapChainTransform;
		scCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		scCI.imageArrayLayers = 1;
		scCI.presentMode = swapChainPresentMode;
		scCI.oldSwapchain = this->m_VkSwapChain;
		scCI.imageColorSpace = this->m_VkSurfaceFormatKHR.colorSpace;
		scCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		scCI.queueFamilyIndexCount = 0;
		scCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		scCI.pQueueFamilyIndices = VK_NULL_HANDLE;
		scCI.clipped = VK_TRUE;

		result = vkCreateSwapchainKHR(*this->m_pVkDevice, &scCI, nullptr, &this->m_VkSwapChain);
		if (result != VK_SUCCESS)
		{
			LOG("[ERROR] Create Swapchain failed!");
		}

		uint32_t numImages;
		result = vkGetSwapchainImagesKHR(*this->m_pVkDevice, this->m_VkSwapChain, &numImages, nullptr);
		VkAssert(result, "Unable to get num of swapchain images");

		this->m_SwapchainImages.resize(numImages);
		result = vkGetSwapchainImagesKHR(*this->m_pVkDevice, this->m_VkSwapChain, &numImages, &this->m_SwapchainImages[0]);
		VkAssert(result, "Unable to get swapchain images");

		this->m_SwapchainImageAmount = numImages;

		this->m_SwapChainImageView.resize(numImages);
		for (int i = 0; i < numImages; ++i)
		{

			VkImageSubresourceRange isr{};
			isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			isr.baseMipLevel = 0;
			isr.levelCount = 1;
			isr.baseArrayLayer = 0;
			isr.layerCount = 1;

			VkComponentMapping cm{};
			cm.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			cm.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			cm.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			cm.a = VK_COMPONENT_SWIZZLE_IDENTITY;


			VkImageViewCreateInfo ivCI{};
			ivCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivCI.image = this->m_SwapchainImages[i];
			ivCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ivCI.format = this->m_VkSurfaceFormatKHR.format;
			ivCI.components = cm;
			ivCI.subresourceRange = isr;

			auto result = vkCreateImageView(*this->m_pVkDevice, &ivCI, nullptr, &this->m_SwapChainImageView[i]);
			VkAssert(result, "Can't create ImageView!");
		}
	}

	CVulkanSwapChain::~CVulkanSwapChain()
	{
	}

	uint32_t CVulkanSwapChain::AcquireImage(VkSemaphore& a_rImageAvaiableSemaphore)
	{
		auto result = vkAcquireNextImageKHR(*this->m_pVkDevice, this->m_VkSwapChain, UINT64_MAX, a_rImageAvaiableSemaphore, VK_NULL_HANDLE, &this->m_BackbufferIndex);

		switch (result)
		{
		case VK_SUCCESS:
		case VK_SUBOPTIMAL_KHR:// Swapchain needs to be rebuild!
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			break; // create new Swapchain cause of Window resize
		default:
			VkAssert(result, "Problem occurred during swap chain image acquisition!");
			break;

		}
		return this->m_BackbufferIndex;
	}

	void CVulkanSwapChain::PresentImage(VkQueue& a_rPresentQueue, VkSemaphore& m_rWaitSemaphore)
	{
		VkPresentInfoKHR pI{};
		pI.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pI.swapchainCount = 1;
		pI.pSwapchains = &this->m_VkSwapChain;
		pI.waitSemaphoreCount = 1;
		pI.pWaitSemaphores = &m_rWaitSemaphore;
		pI.pImageIndices = &this->m_BackbufferIndex;
		vkQueuePresentKHR(a_rPresentQueue, &pI);
	}


	uint32_t CVulkanSwapChain::GetSwapchainImageAmount()
	{
		return this->m_SwapchainImageAmount;
	}

	VkSwapchainKHR * CVulkanSwapChain::GetVkSwapchain()
	{
		return &this->m_VkSwapChain;
	}

	std::vector<VkImage>* CVulkanSwapChain::GetSwapchainImages()
	{
		return &this->m_SwapchainImages;
	}

	std::vector<VkImageView>* CVulkanSwapChain::GetSwapChainImageView()
	{
		return &this->m_SwapChainImageView;
	}

	VkSurfaceFormatKHR * CVulkanSwapChain::GetSurfaceFormat()
	{
		return &this->m_VkSurfaceFormatKHR;
	}

	VkExtent2D CVulkanSwapChain::GetSwapChainExtent() const
	{
		return this->m_SwapChainExtent;
	}
}