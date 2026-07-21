#include "Application.h"
#include <vector>
#include <vulkan/vulkan.h>

//TODO: Swap for agnostic function that checks for any layer support
bool checkValidationLayerSupport(){
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    //We could pass a vector in here instead of a literal string to expand functionality
	for(const auto& layerName : {"VK_LAYER_KHRONOS_validation"}){
		bool layerFound = false;
		for(const auto& layerProperties : availableLayers){
			if(strcmp(layerName, layerProperties.layerName) == 0){
				layerFound = true;
				break;
			}
		}
		if(!layerFound){
			return false;
		}
	}
	return true;
}

void Application::InitializeVulkan(){
	chk(SDL_Init(SDL_INIT_VIDEO));
	chk(SDL_Vulkan_LoadLibrary(NULL));
	volkInitialize();

	//Init Vulkan
	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = name.c_str(),
		.apiVersion = VK_API_VERSION_1_3,
	};

	//Enable validation layers
	if(vulkanContext.enableValidationLayers && !checkValidationLayerSupport()){
		throw std::runtime_error("Validation layers enabled but not available");
	}

	uint32_t instanceExtensionsCount{ 0 };
	char const* const* instanceExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionsCount) };
	VkInstanceCreateInfo instanceCI{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = instanceExtensionsCount,
		.ppEnabledExtensionNames = instanceExtensions
	};
	if(enableValidationLayers){
			instanceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instanceCI.ppEnabledLayerNames = validationLayers.data();
	}
    chk(vkCreateInstance(&instanceCI, nullptr, &vulkanContext.instance));
	volkLoadInstance(instance);

	//Check Queue families for supported features


	//Set up physical device


	//Set up logical device

	
	//Set up VMA


	//Open SDL Window


	//Global Swapchain?
}