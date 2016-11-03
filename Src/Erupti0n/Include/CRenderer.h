#pragma once
#include "Shared.h"
#include "Content\SCContentManager.h"
#include <unordered_map>
#include <array>

#include "CMesh.h"

#include "Vulkan\CVulkanVirtualFrame.h"
#include "Vulkan\CVulkanBufferBase.h"

namespace Erupti0n
{
	class CGraphics;

	struct CRenderCreateInfo
	{
		std::vector<uint32_t> windowIndecies;
		CGraphics* pGraphics;
		VkDevice* pDevice;
	};

	using RenderUnitHandle = uint32_t;

	struct RenderUnit
	{
		CVulkanBufferBase* pVertexBuffer;
		CVulkanBufferBase* pIndexBuffer;
		size_t vertexbufferSize;
		size_t indexbufferSize;

		bool m_isVisible;
	};

	struct ProjectionViewUBO
	{
		glm::mat4 projection;
		glm::mat4 view;
	};

	class CRenderer
	{
	private:

		uint32_t m_virtualFrameCount = 3;
		uint32_t m_virtualFrameId;

		CGraphics* m_pGraphics;
		VkDevice* m_pVkDevice;

		std::vector<uint32_t> m_WindowIndecies;


		std::vector<VkAttachmentDescription> m_VkAttachmentDescriptions;
		std::vector<VkAttachmentReference> m_VkAttachmentReference;
		std::vector<VkSubpassDescription> m_VkSupassDescription;
		std::vector<VkSubpassDependency> m_VkSubpassDependencies;


		std::vector<VkVertexInputBindingDescription> m_VkVertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> m_VkVertexInputAttributeDescription;


		VkRenderPass m_TriangleRenderPass;


		VkShaderModule m_pFragmentShader;
		VkShaderModule m_pVertexShader;

		VkPipeline m_VkPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
		
		VkCommandPool m_vkCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffer;
		

		std::vector<SVulkanVirtualFrame> m_VirtualFrames;
		VkFramebuffer m_FrameBuffer;
		std::vector<VkFramebuffer> m_Framebuffer;


		std::unordered_map<RenderUnitHandle, RenderUnit> m_RenderUnits;


		std::array<float, 3> m_pushConstants;

		VkDescriptorPool m_VkDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayoutBinding m_UniformBufferDescriptorSetLayoutBinding;
		VkDescriptorSetLayout m_VkDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_VkDescriptorSet = VK_NULL_HANDLE;

		CVulkanBufferBase* m_pUniformBuffer;
		VkSampler m_vkSampler;

		VkImageView m_SamplerImageView;

		float timer;

	public:
		CRenderer();
		~CRenderer();

	public:
		void Init(CRenderCreateInfo& a_RenderCreateInfo);
		void Render();
		void Draw();

		RenderUnitHandle AddMesh(CMesh& a_rMesh);
		void BindTexture(CVulkanImage* a_pVulkanImage);

	private:
		void InitRenderpass();
		void LoadShader();
		std::vector<VkPipelineShaderStageCreateInfo> CreateShaderStage();
		void InitVertexLayout();
		void InitDescriptorSetLayout();
		void CreateDescriptorPool();
		void UpdateDescriptorSets();
		void CreatePipeline();
		void CreateCommandPool();
		void InitVirtualFrames();
		void CreateUniformBuffer();
		void PrepareCommandBuffers();
		void CreateImageSampler();


	};
}