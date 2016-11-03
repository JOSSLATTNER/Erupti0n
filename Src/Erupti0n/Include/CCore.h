#pragma once
#include <string>
#include <vector>

#include "Interfaces\IWindow.h"
#include "CGraphics.h"
#include "Shared.h"
#include "CRenderer.h"
#include "CMesh.h"
#include "Vulkan\CVulkanImage.h"
#include "Content\SCContentManager.h"

namespace Erupti0n
{
	class CCore
	{
	private:
		IWindow* m_Window; // Replace with a datastructure for windows
		bool m_isRunning;

		std::vector<IWindow*> m_pWindows;

		std::vector<const char*> m_InstanceLayers;
		std::vector<const char*> m_InstanceExtensions;


		VkInstance m_VkInstance = VK_NULL_HANDLE;
		std::vector<VkPhysicalDevice> m_VkPhysicalDevices;
		std::vector<CGraphics*> m_pGraphics;

		std::vector<VkSwapchainKHR> m_VkSwapchainKHRs;

		CRenderer* m_pRenderer;

		VkDebugReportCallbackEXT m_VkDebugReport = VK_NULL_HANDLE;
		VkDebugReportCallbackCreateInfoEXT drcCI;

	public:
		CCore();
		~CCore();

	public:
		void Run();

		void RegisterWindow(int a_Width, int a_Height, std::string& a_rString);
		void DeregisterWindow(uint32_t a_WindowID);

	private:
		void HandelInput(unsigned int a_KeyCode);

		void InitGraphic();
		void InitDebug();
		
	};

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(VkDebugReportFlagsEXT a_ReportFlags, VkDebugReportObjectTypeEXT a_ObjectType, uint64_t a_srcObjects, size_t a_location, int32_t a_msgCode, const char* a_pLayerPrefix, const char* a_pMessage, void* a_pUserdate);

}