#include "Application.h"
#include <vector>
#include <vulkan/vulkan.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "volk.h"

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
	if(vulkanContext.enableValidationLayers){
			instanceCI.enabledLayerCount = static_cast<uint32_t>(vulkanContext.validationLayers.size());
			instanceCI.ppEnabledLayerNames = vulkanContext.validationLayers.data();
	}
    chk(vkCreateInstance(&instanceCI, nullptr, &vulkanContext.instance));
	volkLoadInstance(vulkanContext.instance);


	//Set up physical device
	//Select Device (First device in list by default)
	uint32_t deviceCount{ 0 };
	chk(vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, nullptr));
	std::vector<VkPhysicalDevice> devices(deviceCount);
	chk(vkEnumeratePhysicalDevices(vulkanContext.instance, &deviceCount, devices.data()));
	uint32_t deviceIndex{ 0 };
	//TODO: More inteligent device selection based on properties
	/*if(argc > 1) {
		deviceIndex = std::stoi(argv[1]);
		assert(deviceIndex < deviceCount);
	}*/
	vulkanContext.physicalDevice = devices[deviceIndex];
	VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	vkGetPhysicalDeviceProperties2(vulkanContext.physicalDevice, &deviceProperties);
	std::cout << "Selected device: " << deviceProperties.properties.deviceName << std::endl;
	//Look through queue families available
	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanContext.physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanContext.physicalDevice, &queueFamilyCount, queueFamilies.data());
	uint32_t queueFamily{ 0 };
	//Select a queueFamily to work on from the device. Only graphics Q for now. Other work queues are available usually.
	//This might drive device selection as well.
	//Anything I want to run at the moment should be alble to run on crummy GPUs just fine.
	//If not, I have failed.
	for(size_t i = 0; i < queueFamilies.size(); i++){
		if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			if(SDL_Vulkan_GetPresentationSupport(vulkanContext.instance, vulkanContext.physicalDevice, i) == VK_TRUE){
				vulkanContext.graphicsQueueFamily = i;
				break;
			}
		}
	}

	//Redundant, but this will assert/crash if a queue wasn't found earlier
	chk(SDL_Vulkan_GetPresentationSupport(vulkanContext.instance, vulkanContext.physicalDevice, queueFamily));
	const float qfpriorities{ 1.0f };
	VkDeviceQueueCreateInfo queueCI{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = vulkanContext.graphicsQueueFamily,
		.queueCount = 1,
		.pQueuePriorities = &qfpriorities
	};
	//Logical device extension and feature selection
	const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_dynamic_rendering", "VK_KHR_synchronization2" };

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{
    	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    	.pNext = nullptr,
    	.dynamicRendering = true
	};
	VkPhysicalDeviceSynchronization2Features sync2Features {
    	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
    	.pNext = &dynamicRenderingFeatures,
    	.synchronization2 = true
	};
	VkPhysicalDeviceVulkan12Features enabledVk12Features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &sync2Features,
		.descriptorIndexing = true,
		.shaderSampledImageArrayNonUniformIndexing = true,
		.descriptorBindingVariableDescriptorCount = true,
		.runtimeDescriptorArray = true,
		.bufferDeviceAddress = true
	};

	VkPhysicalDeviceFeatures enabledVk10Features{
		.samplerAnisotropy = VK_TRUE
	};
	VkDeviceCreateInfo deviceCI{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enabledVk12Features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCI,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &enabledVk10Features
	};
	chk(vkCreateDevice(vulkanContext.physicalDevice, &deviceCI, nullptr, &vulkanContext.device));
	vkGetDeviceQueue(vulkanContext.device, vulkanContext.graphicsQueueFamily, 0, &vulkanContext.graphicsQueue);

	
	//Set up VMA


	//Open SDL Window


	//Global Swapchain?
}