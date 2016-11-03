#include "CRenderer.h"
#include "CGraphics.h"

Erupti0n::CRenderer::CRenderer()
{
}

Erupti0n::CRenderer::~CRenderer()
{
}

void Erupti0n::CRenderer::Init(CRenderCreateInfo& a_rRendererCreateInfo)
{
	this->m_pGraphics = a_rRendererCreateInfo.pGraphics;
	this->m_pVkDevice = a_rRendererCreateInfo.pDevice;
	this->m_WindowIndecies = a_rRendererCreateInfo.windowIndecies;

	this->InitRenderpass();
	this->InitVertexLayout();
	this->InitDescriptorSetLayout();
	this->CreateDescriptorPool();
	this->CreatePipeline();
	this->CreateCommandPool();
	this->InitVirtualFrames();
	this->CreateUniformBuffer();
	this->CreateImageSampler();
}

void Erupti0n::CRenderer::Render()
{
	this->m_virtualFrameId = (this->m_virtualFrameId + 1) % this->m_virtualFrameCount;
	this->timer += 0.0166;

	this->m_pGraphics->Begin(this->m_WindowIndecies, this->m_VirtualFrames[this->m_virtualFrameId].ImageAvaiable);

	this->Draw();

	this->m_pGraphics->End(this->m_VirtualFrames[this->m_virtualFrameId].FrameRenderingFinished);
}

void Erupti0n::CRenderer::Draw()
{
	SVulkanVirtualFrame& rVirtuallFrame = this->m_VirtualFrames[this->m_virtualFrameId];
	//std::cout << "[MESSAGE] -------- Render virtual Frame  : Current Frame ID:" << this->m_virtualFrameId <<"--------" <<std::endl;

	auto resultFenc = vkGetFenceStatus(*this->m_pVkDevice, rVirtuallFrame.FrameFence);
	//std::cout << "[MESSAGE] CheckFence before reset :" << resultFenc << std::endl;

	auto result = vkWaitForFences(*this->m_pVkDevice, 1, &rVirtuallFrame.FrameFence, VK_FALSE, 1000000000);
	if (result == VK_TIMEOUT)
	{
		//std::cout << "[MESSAGE] Wait on Swapchain" << std::endl;
	}
	resultFenc = vkGetFenceStatus(*this->m_pVkDevice, rVirtuallFrame.FrameFence);
	//std::cout << "[MESSAGE] CheckFence while reset :" << resultFenc << std::endl;

	vkResetFences(*this->m_pVkDevice, 1, &rVirtuallFrame.FrameFence);

	resultFenc = vkGetFenceStatus(*this->m_pVkDevice, rVirtuallFrame.FrameFence);
	//std::cout << "[MESSAGE] CheckFence after reset :" << resultFenc << std::endl;


	std::vector<uint32_t> swapchainimages = *this->m_pGraphics->GetSwapChainImageIndecies();
	std::vector<VkImageView> swapchainimageViews = *this->m_pGraphics->GetSwapChainImageViewForWindowIndex(0);

	if (this->m_FrameBuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(*this->m_pVkDevice, this->m_FrameBuffer, nullptr);


	VkExtent2D swapchainext = this->m_pGraphics->GetSwapChainExtent(0);

	VkFramebufferCreateInfo fbCI{};
	fbCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbCI.renderPass = this->m_TriangleRenderPass;
	fbCI.attachmentCount = 1;
	fbCI.pAttachments = &swapchainimageViews[swapchainimages[0]];
	fbCI.width = swapchainext.width;
	fbCI.height = swapchainext.height;
	fbCI.layers = 1;

	result = vkCreateFramebuffer(*this->m_pVkDevice, &fbCI, nullptr, &this->m_FrameBuffer);

	VkCommandBufferBeginInfo cbBI{};
	cbBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	vkBeginCommandBuffer(rVirtuallFrame.FrameCommandBuffer, &cbBI);
	
	VkImageSubresourceRange isr{};
	isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	isr.baseMipLevel = 0;
	isr.levelCount = 1;
	isr.baseArrayLayer = 0;
	isr.layerCount = 1;

	auto sci = m_pGraphics->GetSwapChainImages(0);

	if (*this->m_pGraphics->GetPresentQueue() != *this->m_pGraphics->GetGraphicsQueue())
	{
		VkImageMemoryBarrier pbr{};
		pbr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		pbr.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		pbr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		pbr.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		pbr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		pbr.srcQueueFamilyIndex = this->m_pGraphics->GetPresentationFamilyIndex();
		pbr.dstQueueFamilyIndex = this->m_pGraphics->GetGraphicsQueueFamilyIndex();
		pbr.image = (VkImage)sci[swapchainimages[0]];
		pbr.subresourceRange = isr;

		vkCmdPipelineBarrier(rVirtuallFrame.FrameCommandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &pbr);

	}

	VkClearValue ccv{ {0.08,0.08,0.09} };

	VkRenderPassBeginInfo rBI{};
	rBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rBI.renderPass = this->m_TriangleRenderPass;
	rBI.framebuffer = this->m_FrameBuffer;
	rBI.renderArea.extent = swapchainext;
	rBI.clearValueCount = 1;
	rBI.pClearValues = &ccv;

		vkCmdBeginRenderPass(rVirtuallFrame.FrameCommandBuffer, &rBI, VK_SUBPASS_CONTENTS_INLINE);

		this->m_pushConstants[0] = timer;

		vkCmdPushConstants(rVirtuallFrame.FrameCommandBuffer, this->m_vkPipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(this->m_pushConstants), this->m_pushConstants.data());

		VkViewport v{};
		v.width = swapchainext.width;
		v.height = swapchainext.height;
		v.minDepth = 1.0f;

		VkRect2D s{};
		s.extent.width = swapchainext.width;
		s.extent.height = swapchainext.height;

		vkCmdSetViewport(rVirtuallFrame.FrameCommandBuffer, 0, 1, &v);
		vkCmdSetScissor(rVirtuallFrame.FrameCommandBuffer, 0, 1, &s);

			// Combine and loop
			vkCmdBindPipeline(rVirtuallFrame.FrameCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->m_VkPipeline);
			for (auto ru = this->m_RenderUnits.begin(); ru != this->m_RenderUnits.end(); ++ru)
			{
				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(rVirtuallFrame.FrameCommandBuffer, 0, 1, &ru->second.pVertexBuffer->GetBufferHandle(), &offset);
				vkCmdBindIndexBuffer(rVirtuallFrame.FrameCommandBuffer, ru->second.pIndexBuffer->GetBufferHandle(), offset, VK_INDEX_TYPE_UINT16);
				//vkCmdDraw(rVirtuallFrame.FrameCommandBuffer, 4, 1, 0, 0);
				vkCmdBindDescriptorSets(rVirtuallFrame.FrameCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->m_vkPipelineLayout, 0, 1, &this->m_VkDescriptorSet, 0, nullptr);
				vkCmdDrawIndexed(rVirtuallFrame.FrameCommandBuffer, 6, 1, 0, 0, 0);
			}

		vkCmdEndRenderPass(rVirtuallFrame.FrameCommandBuffer);

		if (*this->m_pGraphics->GetPresentQueue() != *this->m_pGraphics->GetGraphicsQueue())
		{
			VkImageMemoryBarrier pbr;
			pbr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			pbr.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			pbr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			pbr.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			pbr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			pbr.srcQueueFamilyIndex = this->m_pGraphics->GetGraphicsQueueFamilyIndex();
			pbr.dstQueueFamilyIndex = this->m_pGraphics->GetPresentationFamilyIndex();
			pbr.image = (VkImage)sci[swapchainimages[0]];
			pbr.subresourceRange = isr;

			vkCmdPipelineBarrier(rVirtuallFrame.FrameCommandBuffer,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr, 1, &pbr);
		}

	result = vkEndCommandBuffer(rVirtuallFrame.FrameCommandBuffer);
	VkAssert(result, "Faild to end command buffer!");

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo si{};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.waitSemaphoreCount = 1;
	si.pWaitSemaphores = &rVirtuallFrame.ImageAvaiable;
	si.pWaitDstStageMask = &waitDstStageMask;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &rVirtuallFrame.FrameCommandBuffer;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &rVirtuallFrame.FrameRenderingFinished;

	result = vkQueueSubmit(*this->m_pGraphics->GetGraphicsQueue(), 1, &si, rVirtuallFrame.FrameFence);
	//VkAssert(result, std::string("[ERROR] Can't submit command buffer to graphics queue!"));
	//std::cout << "[MESSAGE] -------- Render virtual Frame End : Current Frame ID:" << this->m_virtualFrameId << "--------" << std::endl<<std::endl;
}

Erupti0n::RenderUnitHandle Erupti0n::CRenderer::AddMesh(CMesh& a_rMesh)
{

	RenderUnitHandle handle = HashFNV<CMesh>(a_rMesh);
	if (m_RenderUnits.count(handle) > 0)
	{
		return handle;
	}
	
	//Align vertex data according to vertex layout
	uint32_t numVertices = a_rMesh.m_VertexPosition.size() / 3;
	std::vector<float> vertexData;
	std::vector<uint16_t> indices;
	size_t vertexbuffersize = sizeof(float) * 4 * 2 * numVertices;
	size_t indexbuffersize = sizeof(uint16_t) * a_rMesh.m_VertexIndices.size();

	for (int i = 0; i < numVertices; ++i)
	{
		vertexData.push_back(a_rMesh.m_VertexPosition[i * 3]);
		vertexData.push_back(a_rMesh.m_VertexPosition[i * 3 + 1]);
		vertexData.push_back(a_rMesh.m_VertexPosition[i * 3 + 2]);
		vertexData.push_back(0.0f);

		if (a_rMesh.m_VertexUv.size() > 0)
		{
			vertexData.push_back(a_rMesh.m_VertexUv[i * 2]);
			vertexData.push_back(a_rMesh.m_VertexUv[i * 2 + 1]);
			vertexData.push_back(0.0f);
			vertexData.push_back(0.0f);
		}
		
	}

	if (indexbuffersize > 0)
	{
		indices = a_rMesh.m_VertexIndices;
	}

	//Create vertex buffer
	auto vertexbuffer = m_pGraphics->CreateBuffer(vertexbuffersize,&vertexData[0],true);
	auto indexbuffer = this->m_pGraphics->CreateBuffer(indexbuffersize,&indices[0],true);

	//Create render unit
	RenderUnit unit;
	unit.m_isVisible = true;
	unit.pVertexBuffer = vertexbuffer;
	unit.pIndexBuffer = indexbuffer;
	unit.vertexbufferSize = numVertices;
	unit.indexbufferSize = a_rMesh.m_VertexIndices.size();

	//Add to pipeline
	this->m_RenderUnits[handle] = unit;

	return handle;
}

void Erupti0n::CRenderer::InitRenderpass()
{
	VkAttachmentDescription ad{};
	ad.format = this->m_pGraphics->GetSwapchainFormat();
	ad.samples = VK_SAMPLE_COUNT_1_BIT;
	ad.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ad.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ad.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ad.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ad.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ad.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	this->m_VkAttachmentDescriptions.push_back(ad);

	for (int i = 0; i < this->m_VkAttachmentDescriptions.size(); ++i)
	{
		VkAttachmentReference colorAR{};
		colorAR.attachment = i;
		colorAR.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		this->m_VkAttachmentReference.push_back(colorAR);
	}

	VkSubpassDescription spD{};
	spD.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	spD.colorAttachmentCount = this->m_VkAttachmentDescriptions.size(); // Graphics Att Desc?
	spD.pColorAttachments = &this->m_VkAttachmentReference[0];

	this->m_VkSupassDescription.push_back(spD);

	VkSubpassDependency sd{};
	sd.srcSubpass = VK_SUBPASS_EXTERNAL;
	sd.dstSubpass = 0;
	sd.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sd.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sd.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sd.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sd.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	m_VkSubpassDependencies.push_back(sd);

	sd.srcSubpass = 0;
	sd.dstSubpass = VK_SUBPASS_EXTERNAL;
	sd.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sd.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	sd.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	sd.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	sd.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	m_VkSubpassDependencies.push_back(sd);

	VkRenderPassCreateInfo rpCI{};
	rpCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpCI.attachmentCount = 1;
	rpCI.pAttachments = &m_VkAttachmentDescriptions[0];
	rpCI.subpassCount = 1;
	rpCI.pSubpasses = &this->m_VkSupassDescription[0];
	rpCI.dependencyCount = this->m_VkSubpassDependencies.size();
	rpCI.pDependencies = &this->m_VkSubpassDependencies[0];

	auto result = vkCreateRenderPass(*this->m_pVkDevice, &rpCI, nullptr, &this->m_TriangleRenderPass);
	VkAssert(result, "Can't create Triangle renderpass!");
}

void Erupti0n::CRenderer::LoadShader()
{
	std::vector<char> shaderCoder = TSingleton<SCContentManager>::Instance()->LoadBinaryFile(std::string("spv.vert"));

	if (!shaderCoder.size())
		LOG("[ERROR] Can't load vertex shader!");

	VkShaderModuleCreateInfo smCI{};
	smCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	smCI.codeSize = shaderCoder.size();
	smCI.pCode = reinterpret_cast<const uint32_t*>(&shaderCoder[0]);

	auto result = vkCreateShaderModule(*this->m_pVkDevice, &smCI, nullptr, &this->m_pVertexShader);
	VkAssert(result, "[ERROR] Can't load Vertex Shader!");

	shaderCoder = TSingleton<SCContentManager>::Instance()->LoadBinaryFile(std::string("spv.frag"));

	if (!shaderCoder.size())
		LOG("[ERROR] Can't load vertex shader!");


	smCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	smCI.codeSize = shaderCoder.size();
	smCI.pCode = reinterpret_cast<const uint32_t*>(&shaderCoder[0]);

	result = vkCreateShaderModule(*this->m_pVkDevice, &smCI, nullptr, &this->m_pFragmentShader);
	VkAssert(result, "[ERROR] Can't load Fragment Shader!");

}

std::vector<VkPipelineShaderStageCreateInfo> Erupti0n::CRenderer::CreateShaderStage()
{
	this->LoadShader();

	std::vector<VkPipelineShaderStageCreateInfo> pstCI(2);
	pstCI[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pstCI[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pstCI[0].module = this->m_pVertexShader;
	pstCI[0].pName = "main";

	pstCI[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pstCI[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pstCI[1].module = this->m_pFragmentShader;
	pstCI[1].pName = "main";

	return pstCI;
}

void Erupti0n::CRenderer::InitVertexLayout()
{
	VkVertexInputBindingDescription vibd{};
	vibd.binding = 0;
	vibd.stride = sizeof(float) * 4 * 2;
	vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	this->m_VkVertexInputBindingDescriptions.push_back(vibd);


	VkVertexInputAttributeDescription viad{};
	viad.location = 0;
	viad.binding = this->m_VkVertexInputBindingDescriptions[0].binding;
	viad.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	viad.offset = 0;
	this->m_VkVertexInputAttributeDescription.push_back(viad);

	viad.location = 1;
	viad.binding = this->m_VkVertexInputBindingDescriptions[0].binding;
	viad.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	viad.offset = 16;
	this->m_VkVertexInputAttributeDescription.push_back(viad);
}

void Erupti0n::CRenderer::InitDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding ubodslb{};
	ubodslb.binding = 0;
	ubodslb.descriptorCount = 1;
	ubodslb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubodslb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerdslb{};
	samplerdslb.binding = 1;
	samplerdslb.descriptorCount = 1;
	samplerdslb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerdslb.pImmutableSamplers = nullptr;
	samplerdslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerdslb };
	VkDescriptorSetLayoutCreateInfo dslCI{};
	dslCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dslCI.bindingCount = bindings.size();
	dslCI.pBindings = bindings.data();
	
	auto result = vkCreateDescriptorSetLayout(*this->m_pVkDevice, &dslCI, nullptr, &this->m_VkDescriptorSetLayout);
	VkAssert(result, "Can't create Uniform Descriptor set layout!");
}

void Erupti0n::CRenderer::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 1> poolSize = {};
	//poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//poolSize[0].descriptorCount = 1;
	poolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo dpCI{};
	dpCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpCI.maxSets = 1;
	dpCI.poolSizeCount = poolSize.size();
	dpCI.pPoolSizes = poolSize.data();

	auto result = vkCreateDescriptorPool(*this->m_pVkDevice, &dpCI, nullptr, &this->m_VkDescriptorPool);
	VkAssert(result, "[ERROR] Can't create Descripto pool");
}

void Erupti0n::CRenderer::UpdateDescriptorSets()
{
	VkDescriptorSetLayout layouts[] = { this->m_VkDescriptorSetLayout };
	VkDescriptorSetAllocateInfo dsAI{};
	dsAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAI.descriptorPool = this->m_VkDescriptorPool;
	dsAI.descriptorSetCount = 1;
	dsAI.pSetLayouts = layouts;

	auto result = vkAllocateDescriptorSets(*this->m_pVkDevice, &dsAI, &this->m_VkDescriptorSet);
	VkAssert(result, "[ERROR] Can't allocate Descriptor Set");

	VkDescriptorBufferInfo dBI{};
	dBI.buffer = this->m_pUniformBuffer->GetBufferHandle();
	dBI.range = this->m_pUniformBuffer->GetBufferSize();

	VkDescriptorImageInfo sII{};
	sII.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	sII.imageView = this->m_SamplerImageView;
	sII.sampler = this->m_vkSampler;

	std::array<VkWriteDescriptorSet, 1> descWrites = {};
	/*descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[0].dstSet = this->m_VkDescriptorSet;
	descWrites[0].dstBinding = 0;
	descWrites[0].dstArrayElement = 0;
	descWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descWrites[0].descriptorCount = 1;
	descWrites[0].pBufferInfo = &dBI;*/

	descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[0].dstSet = this->m_VkDescriptorSet;
	descWrites[0].dstBinding = 1;
	descWrites[0].dstArrayElement = 0;
	descWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descWrites[0].descriptorCount = 1;
	descWrites[0].pImageInfo = &sII;

	vkUpdateDescriptorSets(*this->m_pVkDevice, descWrites.size(), descWrites.data(), 0, nullptr);

}

void Erupti0n::CRenderer::CreatePipeline()
{
	VkPipelineVertexInputStateCreateInfo pvisCI{};
	pvisCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pvisCI.vertexBindingDescriptionCount = this->m_VkVertexInputBindingDescriptions.size();
	pvisCI.pVertexBindingDescriptions = &this->m_VkVertexInputBindingDescriptions[0];
	pvisCI.vertexAttributeDescriptionCount = this->m_VkVertexInputAttributeDescription.size();
	pvisCI.pVertexAttributeDescriptions = &this->m_VkVertexInputAttributeDescription[0];
	
	VkPipelineInputAssemblyStateCreateInfo piasCI{};
	piasCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	piasCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	piasCI.primitiveRestartEnable = VK_FALSE;

	//Dynamic Viewport
	VkPipelineViewportStateCreateInfo pvsCI{};
	pvsCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pvsCI.viewportCount = 1;
	pvsCI.scissorCount = 1;

	std::vector<VkDynamicState> ds;
	ds.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	ds.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo pdsCI{};
	pdsCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pdsCI.dynamicStateCount = ds.size();
	pdsCI.pDynamicStates = &ds[0];
	// Dynamic Viewport End

	VkPipelineRasterizationStateCreateInfo prsCI{};
	prsCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	prsCI.depthClampEnable = VK_FALSE;
	prsCI.rasterizerDiscardEnable = VK_FALSE;
	prsCI.polygonMode = VK_POLYGON_MODE_FILL;
	prsCI.cullMode = VK_CULL_MODE_BACK_BIT;
	prsCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
	prsCI.depthBiasEnable = VK_FALSE;
	prsCI.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo pmsCI{};
	pmsCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pmsCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pmsCI.sampleShadingEnable = VK_FALSE;
	pmsCI.minSampleShading = 1.0f;
	pmsCI.alphaToCoverageEnable = VK_FALSE;
	pmsCI.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState pcbas{};
	pcbas.blendEnable = VK_FALSE;
	pcbas.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	pcbas.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	pcbas.colorBlendOp = VK_BLEND_OP_ADD;
	pcbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pcbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pcbas.alphaBlendOp = VK_BLEND_OP_ADD;
	pcbas.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo pcbsCI{};
	pcbsCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pcbsCI.logicOpEnable = VK_FALSE;
	pcbsCI.logicOp = VK_LOGIC_OP_COPY;
	pcbsCI.attachmentCount = 1;
	pcbsCI.pAttachments = &pcbas;
	pcbsCI.blendConstants[0] = 0.0f;
	pcbsCI.blendConstants[1] = 0.0f;
	pcbsCI.blendConstants[2] = 0.0f;
	pcbsCI.blendConstants[3] = 0.0f;

	VkPushConstantRange pcr{};
	pcr.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	pcr.size = sizeof(this->m_pushConstants);

	VkPipelineLayoutCreateInfo plCI{};
	plCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plCI.pushConstantRangeCount = 1;
	plCI.pPushConstantRanges = &pcr;
	plCI.setLayoutCount = 1;
	plCI.pSetLayouts = &this->m_VkDescriptorSetLayout;

	auto result = vkCreatePipelineLayout(*this->m_pVkDevice, &plCI, nullptr, &this->m_vkPipelineLayout);
	VkAssert(result, "[ERROR] Can't create Pipeline layout!");

	std::vector<VkPipelineShaderStageCreateInfo> pstCI =  this->CreateShaderStage();

	VkGraphicsPipelineCreateInfo gpCI{};
	gpCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gpCI.stageCount = pstCI.size();
	gpCI.pStages = &pstCI[0];
	gpCI.pVertexInputState = &pvisCI;
	gpCI.pInputAssemblyState = &piasCI;
	gpCI.pViewportState = &pvsCI;
	gpCI.pRasterizationState = &prsCI;
	gpCI.pMultisampleState = &pmsCI;
	gpCI.pColorBlendState = &pcbsCI;
	gpCI.pDynamicState = &pdsCI;
	gpCI.layout = this->m_vkPipelineLayout;
	gpCI.renderPass = this->m_TriangleRenderPass;
	gpCI.basePipelineHandle = VK_NULL_HANDLE;
	gpCI.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(*this->m_pVkDevice, VK_NULL_HANDLE, 1, &gpCI, nullptr, &this->m_VkPipeline);
	VkAssert(result, "[ERROR] Can't create Graphics Pipeline!");
}

void Erupti0n::CRenderer::CreateCommandPool()
{
	VkCommandPoolCreateInfo cpCI{};
	cpCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	cpCI.queueFamilyIndex = this->m_pGraphics->GetGraphicsQueueFamilyIndex();

	auto result = vkCreateCommandPool(*this->m_pVkDevice, &cpCI, nullptr, &this->m_vkCommandPool);
	VkAssert(result, "[ERROR] Can't create command pool!");
}

void Erupti0n::CRenderer::InitVirtualFrames()
{
	auto siw = *this->m_pGraphics->GetSwapChainImageViewForWindowIndex(0);
	uint32_t numImages = siw.size();


	VkCommandBufferAllocateInfo cbaCI{};
	cbaCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbaCI.commandPool = this->m_vkCommandPool;
	cbaCI.commandBufferCount = numImages;
	cbaCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkSemaphoreCreateInfo sCI{};
	sCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fCI{};
	fCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	this->m_VirtualFrames.resize(this->m_virtualFrameCount);
	for (int i = 0; i < this->m_virtualFrameCount; ++i)
	{
		auto result = vkAllocateCommandBuffers(*this->m_pVkDevice, &cbaCI, &this->m_VirtualFrames[i].FrameCommandBuffer);
		VkAssert(result, "[ERROR] Can't allocate command buffer!");

		result = vkCreateSemaphore(*this->m_pVkDevice, &sCI, nullptr, &this->m_VirtualFrames[i].FrameRenderingFinished);
		VkAssert(result, "[ERROR] Can't create Framerenderingfinished Semaphore!");

		result = vkCreateSemaphore(*this->m_pVkDevice, &sCI, nullptr, &this->m_VirtualFrames[i].ImageAvaiable);
		VkAssert(result, "[ERROR] Can't create Framerenderingfinished Semaphore!");

		result = vkCreateFence(*this->m_pVkDevice, &fCI, nullptr, &this->m_VirtualFrames[i].FrameFence);
		VkAssert(result, "[ERROR] Can't create Frame Fence!");
	}
}

void Erupti0n::CRenderer::CreateUniformBuffer()
{
	struct ubo {
		glm::mat4 view;
		glm::mat4 projection;
	};

	ubo das;
	das.view = glm::mat4(1.0f);
	das.projection = glm::mat4(2.0f);


	this->m_pUniformBuffer = this->m_pGraphics->CreateBuffer(sizeof(ubo),&das,true);
}

void Erupti0n::CRenderer::PrepareCommandBuffers()
{

	//VkCommandBufferBeginInfo cbBI{};
	//cbBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//cbBI.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	//

	//VkClearValue cv{};
	//cv.color = { 1.0f, 1.0f, 1.0f, 0.0f };

	//auto sci = m_pGraphics->GetSwapChainImages(0);

	//for (int i = 0; i < this->m_CommandBuffer.size(); ++i)
	//{
	//
	//	vkBeginCommandBuffer(this->m_CommandBuffer[i], &cbBI);

	//	if (this->m_pGraphics->GetPresentQueue() != this->m_pGraphics->GetGraphicsQueue())
	//	{
	//		VkImageMemoryBarrier pbr{};
	//		pbr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//		pbr.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//		pbr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	//		pbr.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//		pbr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//		pbr.srcQueueFamilyIndex = this->m_pGraphics->GetPresentationFamilyIndex();
	//		pbr.dstQueueFamilyIndex = this->m_pGraphics->GetGraphicsQueueFamilyIndex();
	//		pbr.image = (VkImage)sci[i];
	//		pbr.subresourceRange = isr;

	//		vkCmdPipelineBarrier(this->m_CommandBuffer[i],
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//			0, 0, nullptr, 0, nullptr, 1, &pbr);

	//	}

	//		VkRenderPassBeginInfo rpBI{};
	//		rpBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//		rpBI.renderPass = this->m_TriangleRenderPass;
	//		rpBI.framebuffer = this->m_Framebuffer[i];
	//		rpBI.renderArea = { 0,0,780,557 };
	//		rpBI.clearValueCount = 1;
	//		rpBI.pClearValues = &cv;

	//		vkCmdBeginRenderPass(this->m_CommandBuffer[i], &rpBI, VK_SUBPASS_CONTENTS_INLINE);
	//		vkCmdBindPipeline(this->m_CommandBuffer[i], VK_PIPELINE_BIND_POINT_BEGIN_RANGE, this->m_VkPipeline);
	//		vkCmdDraw(this->m_CommandBuffer[i], 6, 1, 0, 0);
	//		//vkCmdClearColorImage(this->m_CommandBuffer[i], this->m_pGraphics->GetSwapChainImages(0).at(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv.color, 1, &isr);
	//		vkCmdEndRenderPass(this->m_CommandBuffer[i]);

	//	
	//	if (*this->m_pGraphics->GetPresentQueue() != *this->m_pGraphics->GetGraphicsQueue())
	//	{
	//		VkImageMemoryBarrier pbr;
	//		pbr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//		pbr.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//		pbr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	//		pbr.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//		pbr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//		pbr.srcQueueFamilyIndex = this->m_pGraphics->GetGraphicsQueueFamilyIndex();
	//		pbr.dstQueueFamilyIndex = this->m_pGraphics->GetPresentationFamilyIndex();
	//		pbr.image = sci[i];
	//		pbr.subresourceRange = isr;

	//		vkCmdPipelineBarrier(this->m_CommandBuffer[i],
	//			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	//			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	//			0, 0, nullptr, 0, nullptr, 1, &pbr);
	//	}

	//	auto result = vkEndCommandBuffer(this->m_CommandBuffer[i]);
	//	VkAssert(result, "[ERROR] Failed to record command buffer!");


	//}
	

}

void Erupti0n::CRenderer::CreateImageSampler()
{
	VkSamplerCreateInfo sCI{};
	sCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sCI.magFilter = VK_FILTER_LINEAR;
	sCI.minFilter = VK_FILTER_LINEAR;
	sCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sCI.anisotropyEnable = VK_FALSE;
	sCI.maxAnisotropy = 0;
	sCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	sCI.unnormalizedCoordinates = VK_FALSE;
	sCI.compareEnable = VK_FALSE;
	sCI.compareOp = VK_COMPARE_OP_ALWAYS;
	sCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sCI.maxLod = 0;
	
	auto result = vkCreateSampler(*this->m_pVkDevice, &sCI, nullptr, &this->m_vkSampler);
	VkAssert(result, "[ERROR] Can't create Image Sampler");

}

void Erupti0n::CRenderer::BindTexture(CVulkanImage * a_pVulkanImage)
{
	this->m_SamplerImageView = a_pVulkanImage->GetImageView();
	this->UpdateDescriptorSets();
}
