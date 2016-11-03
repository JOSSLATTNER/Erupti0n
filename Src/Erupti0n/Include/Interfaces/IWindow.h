#pragma once
#include <string>
#include "Shared.h"
#include "Vulkan\CVulkanSwapChain.h"

namespace Erupti0n
{
	class CCore;

	struct WindowCreationInfo
	{
		int width;
		int height;
		int id;
		std::string windowTitle;
		CCore* pCore;
		void(Erupti0n::CCore::* pInputCallBack)(unsigned int);
		VkInstance* pVkInstace;
		VkPhysicalDevice* pVkPhysicalDevice;
		VkDevice* pVkDevice;
	};

	class IWindow
	{
	protected:

		int m_Width;
		int m_Height;
		std::string m_WindowTitel;
		bool m_isRunning;
		uint32_t m_WindowID;

		CCore* m_pSystem;
		void(Erupti0n::CCore::* m_pInputCallback) (unsigned int);

		VkPhysicalDevice* m_pVkPhysicalDevice;
		VkDevice* m_pVkDevice;
		VkSurfaceKHR m_VkSurface;
		VkFormat m_VkSurfaceFormat;
		
		CVulkanSwapChain* m_pSwapChain;
		std::vector<VkCommandBuffer> m_PresentCommandBuffer;
		uint32_t m_currentBuffer;

	public:
		virtual ~IWindow();

	public:
		virtual void* GetHandel() const = 0;
		virtual void* GetInstance() const = 0;

		virtual void InitWindow(WindowCreationInfo& a_rWindowCreationInfo) = 0;
		virtual void Update() = 0;
		virtual std::vector<VkImage>* GetWindowTextureArray() = 0;

		virtual uint32_t AcquireNextImage(VkSemaphore& a_rImageAvaiableSemaphore) = 0;
		virtual void PresentImage(VkQueue& a_rPresentQueue, VkSemaphore& m_rWaitSemaphore) const = 0;

		bool IsRunning() const;
		void CreateComandBuffer(VkCommandPool& a_rCommandPool, uint32_t a_QueueFamilieIndex);

		VkCommandBuffer* GetCurrentCommandBuffer();
		VkSurfaceFormatKHR* GetSurfaceFormat();
		std::vector<VkImageView>* GetSwapChainImageView();
		VkExtent2D GetSwapChainExtent();

		uint32_t GetWndowID();

	private:
		virtual void CreateSurface(WindowCreationInfo& a_rWindowCreationInfo) = 0;
		virtual void CreateSwapchain(WindowCreationInfo& a_rWindowCreationInfo) = 0;
		virtual void Shutdown() = 0;
	};
}
