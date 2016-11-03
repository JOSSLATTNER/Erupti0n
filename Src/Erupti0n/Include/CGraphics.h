#pragma once
#include "Shared.h"
#include "Interfaces\IWindow.h"
#include "Vulkan\CVulkanBufferBase.h"

namespace Erupti0n
{
	struct CGrapicsCrationInfo
	{
		int graphicsFamilyIndex;
		int32_t transferFamilyIndex;
		VkPhysicalDevice* pVkPhysicalDevice;
		std::vector<const char*> deviceExtensions;
		std::vector<IWindow*>* ppWindows;
	};

	class CGraphics
	{
	private:
		std::vector<const char*> m_pDeviceExtensions;

		VkPhysicalDevice m_VkPhysicalDevice;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
		VkPhysicalDeviceMemoryProperties m_memoryProperties;
		std::vector<VkQueueFamilyProperties> m_QueueFamiliyProperties;
		VkSurfaceKHR m_Surface;
		VkSurfaceFormatKHR m_SurfaceFormat;
		VkFormat m_Format;
		VkDevice m_VkDevice;

		uint32_t m_GraphicsQueueFamilyIndex;
		uint32_t m_presentQueueFamilyIndex;
		uint32_t m_TransferQueueFamilyIndex;


		VkQueue m_VkPresnetQueue;
		VkQueue m_VkGrapicQueue;
		VkQueue m_VkMemoryTransferQueue;

		VkCommandPool m_VkPresentCommandPool;
		VkCommandBuffer m_pVkGraphicCommandBuffer;

		VkCommandPool m_VkTransferCommandPool;

		std::vector<IWindow*>* m_ppWindows;
		std::vector<std::vector<VkImage>>* m_ppWindowImages;
		std::vector<uint32_t> m_WindowImageHandles;
		std::vector<uint32_t> m_activeWindows;

	public:
		CGraphics(CGrapicsCrationInfo& a_rCGraphicsCreationInfo);
		~CGraphics();

	public:
		void Begin(std::vector<uint32_t>& a_rWindowIndices, VkSemaphore& a_rImageAvaiableSemaphore);
		void Draw();
		void End(VkSemaphore& a_pRenderFinishedSemaphore);
		void Present(VkSemaphore& a_pRenderFinishedSemaphore);

		VkPhysicalDevice* GetVkPhysicalDevice();
		VkDevice* GetVkDevice();
		
		uint32_t GetGraphicsQueueFamilyIndex();
		uint32_t GetPresentationFamilyIndex();

		VkQueue* GetPresentQueue();
		VkQueue* GetGraphicsQueue();
		VkQueue* GetMemoryTransferQueue();
		
		VkCommandPool* GetTransferCommandBuffer();

		VkFormat& GetSwapchainFormat();
		VkExtent2D GetSwapChainExtent(uint32_t a_windowIndex);
		std::vector<VkImage> GetSwapChainImages(uint32_t a_windowIndex);
		std::vector<VkImageView>* GetSwapChainImageViewForWindowIndex(uint32_t const a_rWindowIndecies);
		std::vector<uint32_t>* GetSwapChainImageIndecies();
		

		void OnWindowRegistration();

		CVulkanBufferBase* CreateBuffer(uint32_t a_BufferSize, void* a_pData, bool shouldStage);

	};
}