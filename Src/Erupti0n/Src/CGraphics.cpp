#include "CGraphics.h"

Erupti0n::CGraphics::CGraphics(CGrapicsCrationInfo& a_rCGraphicsCreationInfo)
{
	this->m_ppWindows = a_rCGraphicsCreationInfo.ppWindows;
	this->m_GraphicsQueueFamilyIndex = a_rCGraphicsCreationInfo.graphicsFamilyIndex;
	this->m_TransferQueueFamilyIndex = a_rCGraphicsCreationInfo.transferFamilyIndex;
	this->m_presentQueueFamilyIndex = this->m_GraphicsQueueFamilyIndex;
	this->m_VkPhysicalDevice = *a_rCGraphicsCreationInfo.pVkPhysicalDevice;
	this->m_pDeviceExtensions = a_rCGraphicsCreationInfo.deviceExtensions;

	vkGetPhysicalDeviceProperties(this->m_VkPhysicalDevice, &this->m_PhysicalDeviceProperties);
	vkGetPhysicalDeviceMemoryProperties(this->m_VkPhysicalDevice, &this->m_memoryProperties);

	uint32_t numQueueFamily = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(this->m_VkPhysicalDevice, &numQueueFamily, VK_NULL_HANDLE);

	this->m_QueueFamiliyProperties.resize(numQueueFamily);
	vkGetPhysicalDeviceQueueFamilyProperties(this->m_VkPhysicalDevice, &numQueueFamily, m_QueueFamiliyProperties.data());

	float pQueuePriorities[] = { 1.0f };
	VkDeviceQueueCreateInfo deviceQueueCI{};
	deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCI.queueCount = 1;
	deviceQueueCI.queueFamilyIndex = this->m_GraphicsQueueFamilyIndex;
	deviceQueueCI.pQueuePriorities = pQueuePriorities;

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.shaderClipDistance = VK_TRUE;
	deviceFeatures.shaderCullDistance = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;
	deviceFeatures.shaderTessellationAndGeometryPointSize = VK_TRUE;

	VkDeviceCreateInfo deviceCI{};
	deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCI.queueCreateInfoCount = 1;
	deviceCI.pQueueCreateInfos = &deviceQueueCI;
	deviceCI.enabledExtensionCount = (uint32_t)this->m_pDeviceExtensions.size();
	deviceCI.ppEnabledExtensionNames = this->m_pDeviceExtensions.data();
	deviceCI.pEnabledFeatures = &deviceFeatures;

	auto result = vkCreateDevice(this->m_VkPhysicalDevice, &deviceCI, VK_NULL_HANDLE, &this->m_VkDevice);
	if (result != VK_SUCCESS)
	{
		LOG("ERROR: vkCreateDevice() failed!");
	}

	vkGetDeviceQueue(this->m_VkDevice, this->m_GraphicsQueueFamilyIndex, 0, &this->m_VkPresnetQueue);
	vkGetDeviceQueue(this->m_VkDevice, this->m_GraphicsQueueFamilyIndex, 0, &this->m_VkGrapicQueue);
	vkGetDeviceQueue(this->m_VkDevice, this->m_TransferQueueFamilyIndex, 0, &this->m_VkMemoryTransferQueue);


	VkCommandPoolCreateInfo cpCI{};
	cpCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpCI.queueFamilyIndex = this->m_GraphicsQueueFamilyIndex;
	result = vkCreateCommandPool(this->m_VkDevice, &cpCI, nullptr, &this->m_VkPresentCommandPool);
	VkAssert(result, "[ERROR] Can't create Present Command Pool!");

	cpCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpCI.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	cpCI.queueFamilyIndex = this->m_TransferQueueFamilyIndex;
	result = vkCreateCommandPool(this->m_VkDevice, &cpCI, nullptr, &this->m_VkTransferCommandPool);
	VkAssert(result, "[ERROR] Can't create Present Command Pool!");


	VkSemaphoreCreateInfo sCI{};
	sCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (result != VK_SUCCESS)
	{
		LOG("ERROR: Creating RenderFinished Semaphore failed!");
	}

	this->m_ppWindowImages = new std::vector<std::vector<VkImage>>();
}

Erupti0n::CGraphics::~CGraphics()
{

}

void Erupti0n::CGraphics::Begin(std::vector<uint32_t>& a_rWindowIndices, VkSemaphore& a_rImageAvaiableSemaphore)
{
	this->m_activeWindows = a_rWindowIndices;
	this->m_WindowImageHandles.clear();

	if ((*this->m_ppWindows).size() <  a_rWindowIndices.size())
		return; 

	for (int i = 0; i < a_rWindowIndices.size(); ++i)
	{
		
		this->m_WindowImageHandles.push_back(this->m_ppWindows->at(a_rWindowIndices[i])->AcquireNextImage(a_rImageAvaiableSemaphore));
	}
}

void Erupti0n::CGraphics::Draw()
{
}

void Erupti0n::CGraphics::End(VkSemaphore& a_rRenderFinishedSemaphore)
{
	//VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;


	//uint32_t numWindows = this->m_activeWindows.size();
	//std::vector<VkCommandBuffer*> windowBuffer(numWindows);

	//for (int i = 0; i < numWindows; ++i)
	//{
	//	windowBuffer[i] = this->m_ppWindows->at(this->m_activeWindows[i])->GetCurrentCommandBuffer();
	//}


	//VkSubmitInfo sI{};
	//sI.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//sI.waitSemaphoreCount = 1;
	//sI.pWaitSemaphores = this->m_ppWindows->at(0)->GetImageAvailableSemaphore();
	//sI.pWaitDstStageMask = &waitDstStageMask;
	//sI.commandBufferCount = numWindows;
	//sI.pCommandBuffers = windowBuffer[0];
	//sI.signalSemaphoreCount = 1;
	//sI.pSignalSemaphores = &this->m_RenderFinishedVkSemaphore;

	//auto result = vkQueueSubmit(this->m_VkPresnetQueue, 1, &sI, VK_NULL_HANDLE);
	//VkAssert(result, "[ERROR] Can#t submit to Present Queue");

	this->Present(a_rRenderFinishedSemaphore);
}

void Erupti0n::CGraphics::Present(VkSemaphore& a_rRenderFinishedSemaphore)
{
	for (IWindow* window : *this->m_ppWindows)
	{
		window->PresentImage(this->m_VkPresnetQueue, a_rRenderFinishedSemaphore);
	}
}

VkPhysicalDevice* Erupti0n::CGraphics::GetVkPhysicalDevice()
{
	return &this->m_VkPhysicalDevice;
}

VkDevice * Erupti0n::CGraphics::GetVkDevice()
{
	return &this->m_VkDevice;
}

VkFormat& Erupti0n::CGraphics::GetSwapchainFormat()
{
	return this->m_SurfaceFormat.format;
}

uint32_t Erupti0n::CGraphics::GetGraphicsQueueFamilyIndex()
{
	return this->m_GraphicsQueueFamilyIndex;
}

uint32_t Erupti0n::CGraphics::GetPresentationFamilyIndex()
{
	return m_presentQueueFamilyIndex;
}

VkQueue* Erupti0n::CGraphics::GetPresentQueue()
{
	return &m_VkPresnetQueue;
}

VkQueue* Erupti0n::CGraphics::GetGraphicsQueue()
{
	return &m_VkGrapicQueue;
}

VkQueue * Erupti0n::CGraphics::GetMemoryTransferQueue()
{
	return &this->m_VkMemoryTransferQueue;
}

VkCommandPool * Erupti0n::CGraphics::GetTransferCommandBuffer()
{
	return &this->m_VkTransferCommandPool;
}

VkExtent2D Erupti0n::CGraphics::GetSwapChainExtent(uint32_t a_windowIndex)
{
	return this->m_ppWindows->at(a_windowIndex)->GetSwapChainExtent();
}

std::vector<VkImage> Erupti0n::CGraphics::GetSwapChainImages(uint32_t a_windowIndex)
{
	return this->m_ppWindowImages->at(a_windowIndex);
}

std::vector<VkImageView>* Erupti0n::CGraphics::GetSwapChainImageViewForWindowIndex(uint32_t const a_rWindowIndex)
{
	return this->m_ppWindows->at(a_rWindowIndex)->GetSwapChainImageView();
}

std::vector<uint32_t>* Erupti0n::CGraphics::GetSwapChainImageIndecies()
{
	return &this->m_WindowImageHandles;
}

void Erupti0n::CGraphics::OnWindowRegistration()
{
	this->m_ppWindowImages->clear();
	for (int i = 0; i < this->m_ppWindows->size(); ++i)
	{
		this->m_ppWindowImages->push_back(*this->m_ppWindows->at(i)->GetWindowTextureArray());
		this->m_ppWindows->at(i)->CreateComandBuffer(this->m_VkPresentCommandPool,this->m_GraphicsQueueFamilyIndex);
		this->m_SurfaceFormat = *this->m_ppWindows->at(i)->GetSurfaceFormat();
	}
}

Erupti0n::CVulkanBufferBase* Erupti0n::CGraphics::CreateBuffer(uint32_t a_BufferSize, void * a_pData, bool shouldStage)
{
	CVulkanBufferBaseCreateInfo vvbCI{};
	vvbCI.pVkDevice = &this->m_VkDevice;
	vvbCI.pVkPhysicalDevice = &this->m_VkPhysicalDevice;
	vvbCI.pCommandBufferpool = &this->m_VkTransferCommandPool;
	vvbCI.pQueue = &this->m_VkMemoryTransferQueue;

	vvbCI.pData = a_pData;
	vvbCI.size = a_BufferSize;
	vvbCI.useStagingBuffer = shouldStage;

	CVulkanBufferBase* pBuffer = new CVulkanBufferBase(vvbCI);
	return pBuffer;
}
