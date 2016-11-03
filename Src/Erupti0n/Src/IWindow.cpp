#include "Interfaces\IWindow.h"

namespace Erupti0n
{
	IWindow::~IWindow()
	{
	}

	bool IWindow::IsRunning() const
	{
		return this->m_isRunning;
	}

	void IWindow::CreateComandBuffer(VkCommandPool& a_rCommandPool, uint32_t a_QueueFamilieIndex)
	{
		uint32_t numImages = this->m_pSwapChain->GetSwapchainImageAmount();
		this->m_PresentCommandBuffer.resize(numImages);

		VkCommandBufferAllocateInfo cbAI{};
		cbAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbAI.commandPool =  a_rCommandPool;
		cbAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbAI.commandBufferCount = numImages;

		auto result = vkAllocateCommandBuffers(*this->m_pVkDevice, &cbAI, &this->m_PresentCommandBuffer[0]);
		VkAssert(result, "[ERROR] Can't create Present Command Buffer !");

		VkCommandBufferBeginInfo cbbI{};
		cbbI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cbbI.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VkImageSubresourceRange imageSubResRange{};
		imageSubResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubResRange.levelCount = 1;
		imageSubResRange.layerCount = 1;

		std::vector<VkImage>* swpachain_Images = this->m_pSwapChain->GetSwapchainImages();
		for (int i = 0; i < numImages; ++i)
		{
			VkImageMemoryBarrier memBarRendertoPres{};
			memBarRendertoPres.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memBarRendertoPres.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			memBarRendertoPres.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			memBarRendertoPres.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			memBarRendertoPres.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			memBarRendertoPres.srcQueueFamilyIndex = a_QueueFamilieIndex;
			memBarRendertoPres.dstQueueFamilyIndex = a_QueueFamilieIndex;
			memBarRendertoPres.image = swpachain_Images->at(i);
			memBarRendertoPres.subresourceRange = imageSubResRange;

			VkImageMemoryBarrier memBarPrestoRender{};
			memBarPrestoRender.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memBarPrestoRender.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			memBarPrestoRender.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			memBarPrestoRender.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			memBarPrestoRender.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			memBarPrestoRender.srcQueueFamilyIndex = a_QueueFamilieIndex;
			memBarPrestoRender.dstQueueFamilyIndex = a_QueueFamilieIndex;
			memBarPrestoRender.image = swpachain_Images->at(i);
			memBarPrestoRender.subresourceRange = imageSubResRange;

			VkClearColorValue clear_color = {
			{ 0.05f, 0.05f, 0.05f, 0.0f }
			};


			vkBeginCommandBuffer(this->m_PresentCommandBuffer[i], &cbbI);
				vkCmdPipelineBarrier(this->m_PresentCommandBuffer[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 0, &memBarPrestoRender);
				vkCmdClearColorImage(this->m_PresentCommandBuffer[i], swpachain_Images->at(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &imageSubResRange);
				vkCmdPipelineBarrier(this->m_PresentCommandBuffer[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0, &memBarRendertoPres);
			result = vkEndCommandBuffer(this->m_PresentCommandBuffer[i]);
		}
	}

	VkCommandBuffer* IWindow::GetCurrentCommandBuffer()
	{
		return &this->m_PresentCommandBuffer[this->m_currentBuffer];
	}

	VkSurfaceFormatKHR* IWindow::GetSurfaceFormat()
	{
		return this->m_pSwapChain->GetSurfaceFormat();
	}

	std::vector<VkImageView>* IWindow::GetSwapChainImageView()
	{
		return this->m_pSwapChain->GetSwapChainImageView();
	}

	VkExtent2D IWindow::GetSwapChainExtent()
	{
		return this->m_pSwapChain->GetSwapChainExtent();
	}

	uint32_t IWindow::GetWndowID()
	{
		return this->m_WindowID;
	}
}