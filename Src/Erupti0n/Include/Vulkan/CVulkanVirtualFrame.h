#pragma once
#include "Shared.h"

namespace Erupti0n
{
	struct SVulkanVirtualFrame
	{
		VkCommandBuffer FrameCommandBuffer;
		VkFence FrameFence;
		VkSemaphore FrameRenderingFinished;
		VkSemaphore ImageAvaiable;
	};
}