#include "CCore.h"
#include "CWindowWin32.h"
#include "vulkan\vulkan.h"
namespace Erupti0n
{
	CCore::CCore()
	{
		LOG("Core Initializing...");

		// API weg kapseln ? 
		// InitApi()
		drcCI = {};

		this->m_InstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
		this->m_InstanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		this->m_InstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		this->m_InstanceLayers.push_back("VK_LAYER_GOOGLE_threading");
		this->m_InstanceLayers.push_back("VK_LAYER_LUNARG_swapchain");
		this->m_InstanceLayers.push_back("VK_LAYER_LUNARG_image");
		this->m_InstanceLayers.push_back("VK_LAYER_LUNARG_object_tracker");
		this->m_InstanceLayers.push_back("VK_LAYER_LUNARG_parameter_validation");

		this->m_InstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

		drcCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		drcCI.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
		drcCI.pfnCallback = &DebugCallBack;

		VkApplicationInfo application_info{};
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		application_info.pApplicationName = "Erupti0n - v0.0.1";

		std::vector<const char*> instanceLayer;
		std::vector<const char*> instanceExtensions;

		VkDebugReportCallbackCreateInfoEXT* pp = &this->drcCI;

		VkInstanceCreateInfo instanceCI{};
		instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCI.pApplicationInfo = &application_info;
		instanceCI.enabledLayerCount = (uint32_t)this->m_InstanceLayers.size();
		instanceCI.enabledExtensionCount = (uint32_t)this->m_InstanceExtensions.size();
		instanceCI.ppEnabledLayerNames = this->m_InstanceLayers.data();
		instanceCI.ppEnabledExtensionNames = this->m_InstanceExtensions.data();
		instanceCI.pNext = pp;

		auto err = vkCreateInstance(&instanceCI, nullptr, &this->m_VkInstance);

		if (VK_SUCCESS != err)
		{
			assert(0 && "Vulkan Error: Create Instance faild!");
			std::exit(-1);
		}

		this->InitGraphic();
	}

	CCore::~CCore()
	{
	}

	//Non block run
	void CCore::Run()
	{

		std::vector<uint32_t> windows{ 0 };
		this->m_pRenderer = new CRenderer();
		CRenderCreateInfo rCI{};
		rCI.windowIndecies = windows;
		rCI.pGraphics = this->m_pGraphics[0];
		rCI.pDevice = this->m_pGraphics[0]->GetVkDevice();
		this->m_pRenderer->Init(rCI);


		CMesh mesh01;                                  // 0                   1                2                  3
		mesh01.m_VertexPosition = std::vector<float>{ -0.5f,0.5f,0.0f,  -0.5f,-0.5f,0.0f,  0.5f,0.5f,0.0f, 0.5f,-0.5f,0.0f };
		mesh01.m_VertexUv = std::vector<float>{ 0,0, 0,1, 1,0 ,1,1};
		mesh01.m_VertexIndices = std::vector<uint16_t>{ 3,2,1, 1,2,0};

		SLoadTextureInfo ltI{};
		ltI.a_rFilePath = std::string("uvLayoutGrid.jpg");
		ltI.pVkDevice = this->m_pGraphics[0]->GetVkDevice();
		ltI.pVkPhysicalDevice = this->m_pGraphics[0]->GetVkPhysicalDevice();
		ltI.pVkCommandBufferPool = this->m_pGraphics[0]->GetTransferCommandBuffer();
		ltI.pVkTransferQueue = this->m_pGraphics[0]->GetMemoryTransferQueue();

		CVulkanImage image01 = Erupti0n::TSingleton<Erupti0n::SCContentManager>::Instance()->LoadTexture(ltI);
		

		this->m_pRenderer->AddMesh(mesh01);
		this->m_pRenderer->BindTexture(&image01);

		if (!this->m_isRunning)
		{
			this->m_isRunning = true;
			while (this->m_isRunning)
			{
				for (int i = 0; i < this->m_pWindows.size(); ++i)
				{
					this->m_pWindows[i]->Update();
					
				}
				if(this->m_pWindows.size() > 0)
					this->m_pRenderer->Render();
			}
		}
	}

	//Platform indipentend
	//Register in a Datastructure
	void CCore::RegisterWindow(int a_Width, int a_Height, std::string& a_rString)
	{
		this->m_Window = new CWindowWin32();
		this->m_pWindows.push_back(this->m_Window);

		WindowCreationInfo wCI{};
		wCI.width = a_Width;
		wCI.height = a_Height;
		wCI.id = m_pWindows.size() - 1;
		wCI.windowTitle = a_rString;
		wCI.pCore = this;
		wCI.pInputCallBack = &CCore::HandelInput;
		wCI.pVkInstace = &this->m_VkInstance;
		wCI.pVkPhysicalDevice = &this->m_VkPhysicalDevices[0];
		wCI.pVkDevice = this->m_pGraphics[0]->GetVkDevice();
		this->m_Window->InitWindow(wCI);

		for (CGraphics* graphic : this->m_pGraphics)
		{
			graphic->OnWindowRegistration();
		}
	}
	//Remove from dataset
	void CCore::DeregisterWindow(uint32_t a_WindowID)
	{
		
		delete(this->m_pWindows[a_WindowID]);
		this->m_pWindows.erase(this->m_pWindows.begin() + a_WindowID);
		this->m_pGraphics[0]->OnWindowRegistration();
	}

	//Messanging System for comunication between window and Core
	void CCore::HandelInput(unsigned int a_KeyCode)
	{
		printf("%i\n", a_KeyCode);
	}

	void CCore::InitGraphic()
	{


		//Gather gpu count
		uint32_t gpu_count = 0;
		vkEnumeratePhysicalDevices(this->m_VkInstance, &gpu_count, nullptr);

		//Gather List of Physical devices
		std::vector<VkPhysicalDevice> gpu_list(gpu_count);
		vkEnumeratePhysicalDevices(this->m_VkInstance, &gpu_count, gpu_list.data());
		this->m_VkPhysicalDevices = gpu_list;


		for (auto &vkPhysicalDevice : this->m_VkPhysicalDevices)
		{
			std::vector<int> graphicFamIndex;
			std::vector<int> computeFamIndex;
			std::vector<int> transferFamIndex;
			std::vector<int> sparseBindingFamIndex;

			VkQueue Queue = VK_NULL_HANDLE;
			VkDevice Device = VK_NULL_HANDLE;



			uint32_t numLayers = 0;
			vkEnumerateInstanceLayerProperties(&numLayers, nullptr);
			std::vector<VkLayerProperties> layerProperties(numLayers);
			vkEnumerateInstanceLayerProperties(&numLayers, &layerProperties[0]);

			uint32_t numDevicesLayers = 0;
			vkEnumerateDeviceLayerProperties(vkPhysicalDevice, &numDevicesLayers, nullptr);
			std::vector<VkLayerProperties> devicelayerProperties(numDevicesLayers);
			vkEnumerateDeviceLayerProperties(vkPhysicalDevice, &numDevicesLayers,&devicelayerProperties[0]);



			//gather information about the PU
			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(vkPhysicalDevice, &props);

			uint32_t familie_count;
			vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &familie_count, nullptr);

			std::vector<VkQueueFamilyProperties> family_property_list(familie_count);
			vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &familie_count, family_property_list.data());
			bool found = false;
			for (uint32_t i = 0; i < family_property_list.size(); ++i)
			{
				if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
				{
					graphicFamIndex.push_back(i);
				}
				if (family_property_list[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
					transferFamIndex.push_back(i);
			}

			std::vector<const char*> deviceExtensions;
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			CGrapicsCrationInfo gCI{};
			gCI.graphicsFamilyIndex = graphicFamIndex[0];
			gCI.transferFamilyIndex = transferFamIndex[0];
			gCI.pVkPhysicalDevice = &vkPhysicalDevice;
			gCI.deviceExtensions = deviceExtensions;
			gCI.ppWindows = &this->m_pWindows;
			
			this->m_pGraphics.push_back(new CGraphics(gCI));
		}
	}

	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;
	void CCore::InitDebug()
	{

		fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(this->m_VkInstance, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(this->m_VkInstance, "vkDestroyDebugReportCallbackEXT");

		if (!fvkCreateDebugReportCallbackEXT || !fvkDestroyDebugReportCallbackEXT)
			LOG("[ERROR] Can't get function pointer");


	
		
		fvkCreateDebugReportCallbackEXT(this->m_VkInstance, &drcCI, nullptr, &this->m_VkDebugReport);
	}
	
	
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(VkDebugReportFlagsEXT a_ReportFlags, VkDebugReportObjectTypeEXT a_ObjectType, uint64_t a_srcObjects, size_t a_location, int32_t a_msgCode, const char * a_pLayerPrefix, const char * a_pMessage, void * a_pUserdate)
	{
		if (a_ReportFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			std::cout << "[DEBUG INFO]";
		if (a_ReportFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			std::cout << "[DEBUG WARNING]";
		if (a_ReportFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			std::cout << "[DEBUG PERFORMANCE_WARNING]";
		if (a_ReportFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			std::cout << "[DEBUG ERROR]";
		if (a_ReportFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			std::cout << "[DEBUG DEBUG]";
		if (a_ReportFlags & VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT)
			std::cout << "[DEBUG BITS_MAX]";

		std::cout << "(" << a_pLayerPrefix << ")";
		std::cout << a_pMessage << std::endl;
		return false;
	}

}

