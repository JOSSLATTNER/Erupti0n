#include "CWindowWin32.h"
#include "CCore.h"

namespace Erupti0n
{
	CWindowWin32::CWindowWin32()
	{
	}

	CWindowWin32::~CWindowWin32()
	{
		this->Shutdown();
		LOG("Delete Window Win32");
	}

	void CWindowWin32::InitWindow(WindowCreationInfo& a_rWindowCreationInfo)
	{

		this->m_Width = a_rWindowCreationInfo.width;
		this->m_Height = a_rWindowCreationInfo.height;
		this->m_WindowTitel = a_rWindowCreationInfo.windowTitle;
		this->m_pSystem = a_rWindowCreationInfo.pCore;
		this->m_pInputCallback = a_rWindowCreationInfo.pInputCallBack;
		this->m_pVkPhysicalDevice = a_rWindowCreationInfo.pVkPhysicalDevice;
		this->m_pVkDevice = a_rWindowCreationInfo.pVkDevice;
		
		WNDCLASSEX wc;

		this->m_WindowInstance = GetModuleHandle(NULL);

		if (!GetClassInfoEx(this->m_WindowInstance, this->m_WindowTitel.c_str(), &wc))
		{
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = &OwnWindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = this->m_WindowInstance;
			wc.hIcon = NULL;
			wc.hIconSm = wc.hIcon;
			wc.hCursor = NULL;
			wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			wc.lpszMenuName = NULL;
			wc.lpszClassName = this->m_WindowTitel.c_str();
			wc.cbSize = sizeof(WNDCLASSEX);

			bool isRegistrated = RegisterClassEx(&wc);

			if (!isRegistrated)
			{
				LOG(" [Error] Window Registration failed!");
				return;
			}
		}
		this->m_WindowHandel = CreateWindowEx(WS_EX_CLIENTEDGE, this->m_WindowTitel.c_str(), this->m_WindowTitel.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, this->m_Width, this->m_Height, NULL, NULL, this->m_WindowInstance, this);

		if (!this->m_WindowHandel)
		{
			LOG(" [Error] Window Creation failed!");
			return;
		}

		this->CreateSurface(a_rWindowCreationInfo);

		ShowWindow(this->m_WindowHandel, SW_SHOW);
		UpdateWindow(this->m_WindowHandel);
		SetForegroundWindow(this->m_WindowHandel);
		SetFocus(this->m_WindowHandel);
		ShowCursor(true);

		this->m_isRunning = true;

		LOG("Window Created!");
	}

	void CWindowWin32::Update()
	{
		MSG message;
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (!this->m_isRunning)
			this->m_pSystem->DeregisterWindow(this->m_WindowID);
	}

	void * CWindowWin32::GetHandel() const
	{
		return this->m_WindowHandel;
	}
	void * CWindowWin32::GetInstance() const
	{
		return this->m_WindowInstance;
	}

	std::vector<VkImage>* CWindowWin32::GetWindowTextureArray()
	{
		return this->m_pSwapChain->GetSwapchainImages();
	}

	uint32_t CWindowWin32::AcquireNextImage(VkSemaphore& a_rImageAvaiableSemaphore)
	{
		this->m_currentBuffer = this->m_pSwapChain->AcquireImage(a_rImageAvaiableSemaphore);
		return this->m_currentBuffer;
	}

	void CWindowWin32::PresentImage(VkQueue& a_rPresentQueue, VkSemaphore& m_rWaitSemaphore) const
	{
		this->m_pSwapChain->PresentImage(a_rPresentQueue, m_rWaitSemaphore);
	}

	void CWindowWin32::CreateSurface(WindowCreationInfo& a_rWindowCreationInfo)
	{

		VkWin32SurfaceCreateInfoKHR win32SurfaceCI{};
		win32SurfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32SurfaceCI.hinstance = this->m_WindowInstance;
		win32SurfaceCI.hwnd = this->m_WindowHandel;

		auto result = vkCreateWin32SurfaceKHR(*a_rWindowCreationInfo.pVkInstace, &win32SurfaceCI, nullptr, &this->m_VkSurface);
		if (result != VK_SUCCESS)
		{
			LOG("[ERROR] Couldn't create surface!");
		}

		uint32_t numFormats;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(*this->m_pVkPhysicalDevice, this->m_VkSurface, &numFormats, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
		{
			LOG("[ERROR] Couldn't get surface formats!");
		}

		VkSurfaceFormatKHR * pSurfaceFormats = new VkSurfaceFormatKHR[numFormats];
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(*this->m_pVkPhysicalDevice, this->m_VkSurface, &numFormats, pSurfaceFormats);

		if (numFormats == 1 && pSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
			this->m_VkSurfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;
		else
			this->m_VkSurfaceFormat = pSurfaceFormats[0].format;

		this->CreateSwapchain(a_rWindowCreationInfo);
	}

	void CWindowWin32::CreateSwapchain(WindowCreationInfo & a_rWindowCreationInfo)
	{

		VulkanSwapChainCreationInfo vscCI{};
		vscCI.width = a_rWindowCreationInfo.width;
		vscCI.height = a_rWindowCreationInfo.height;
		vscCI.pVkInstace = a_rWindowCreationInfo.pVkInstace;
		vscCI.pVkPhysicalDevice = a_rWindowCreationInfo.pVkPhysicalDevice;
		vscCI.pVkDevice = a_rWindowCreationInfo.pVkDevice;
		vscCI.pVkSurfaceKHR = &this->m_VkSurface;

		this->m_pSwapChain = new CVulkanSwapChain(vscCI);
		//Error checks!! ASAP.. maybe. pls.

	}

	void CWindowWin32::Shutdown()
	{
		DestroyWindow(this->m_WindowHandel);
		UnregisterClass(this->m_WindowTitel.c_str(), this->m_WindowInstance);
		this->m_WindowHandel = NULL;
		this->m_isRunning = false;
	}

	LRESULT CALLBACK CWindowWin32::OwnWindowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
	{
		CWindowWin32* pWindow;

		if (umessage == WM_NCCREATE)
		{
			CREATESTRUCT* cs = (CREATESTRUCT*)lparam;
			pWindow = (CWindowWin32*)((LPCREATESTRUCT)lparam)->lpCreateParams;

			SetLastError(0);

			if (SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWindow) == 0)
				if (GetLastError() != 0)
					return FALSE;
		}
		else
		{
			pWindow = (CWindowWin32*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}

		if (pWindow)
			return pWindow->WindowProc(hwnd, umessage, wparam, lparam);

		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}

	LRESULT CALLBACK CWindowWin32::WindowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
	{
		switch (umessage)
		{
		case WM_DESTROY:
		case WM_CLOSE:
			PostQuitMessage(0);
				this->m_isRunning = false;
				this->Shutdown();
			return 0;
		case WM_KEYDOWN:
			(this->m_pSystem->*this->m_pInputCallback)(wparam);
		default:
			return DefWindowProc(hwnd, umessage, wparam, lparam);
		}
	}
}