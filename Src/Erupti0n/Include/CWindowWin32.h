#pragma once
#include <string>
#include <Windows.h>

#include "Utilities.h"
#include "Interfaces\IWindow.h"

namespace Erupti0n
{
	class CCore;

	class CWindowWin32 : public IWindow
	{
	private:
		HWND m_WindowHandel;
		HINSTANCE m_WindowInstance;
		HDC m_DeviceContext;

	public:
		CWindowWin32();
		~CWindowWin32();

	public:
		
		void InitWindow(WindowCreationInfo& a_rWindowCreationWindow) override;
		void Update() override;

		void* GetHandel() const override;
		void* GetInstance() const override;

		std::vector<VkImage>* GetWindowTextureArray() override;

		uint32_t AcquireNextImage(VkSemaphore& a_rImageAvaiableSemaphore) override;
		void PresentImage(VkQueue& a_rPresentQueue, VkSemaphore& m_rWaitSemaphore) const override;

	private:
		void CreateSurface(WindowCreationInfo& a_rWindowCreationInfo) override;
		void CreateSwapchain(WindowCreationInfo& a_rWindowCreationInfo) override;
		void Shutdown() override;

		LRESULT CALLBACK WindowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
		static LRESULT CALLBACK OwnWindowProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
	};
}