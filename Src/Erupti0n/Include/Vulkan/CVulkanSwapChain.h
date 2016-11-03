#pragma once
#include "Shared.h"
#include <vector>

namespace Erupti0n
{
	struct VulkanSwapChainCreationInfo
	{
		uint32_t height;
		uint32_t width;
		VkInstance* pVkInstace;
		VkPhysicalDevice* pVkPhysicalDevice;
		VkDevice* pVkDevice;
		VkSurfaceKHR* pVkSurfaceKHR;
		VkQueue* pPresentQueue;
	};

	class CVulkanSwapChain
	{
	private:
		VkInstance* m_pVkInstace;
		VkPhysicalDevice* m_pVkPhysicalDevice;
		VkDevice* m_pVkDevice;

		VkSurfaceKHR* m_pSurefaceKHR;		
		VkSurfaceFormatKHR m_VkSurfaceFormatKHR;

		uint32_t m_BackbufferIndex;
		uint32_t m_SwapchainImageAmount;
		VkSwapchainKHR m_VkSwapChain;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapChainImageView;
		
		VkExtent2D m_SwapChainExtent;

	public:
		CVulkanSwapChain(VulkanSwapChainCreationInfo& a_rVulkanSwapChainCreationInfo);
		~CVulkanSwapChain();

		uint32_t AcquireImage(VkSemaphore& a_rImageAvaiableSemaphore);
		void PresentImage(VkQueue& a_rPresentQueue, VkSemaphore& m_rWaitSemaphore);

		uint32_t GetSwapchainImageAmount();
		VkSwapchainKHR* GetVkSwapchain();
		std::vector<VkImage>* GetSwapchainImages();
		std::vector<VkImageView>* GetSwapChainImageView();

		VkSurfaceFormatKHR* GetSurfaceFormat();
		VkExtent2D GetSwapChainExtent() const;
	};
}