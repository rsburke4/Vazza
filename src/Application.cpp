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
	VmaVulkanFunctions vkFunctions{
		.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
		.vkGetDeviceProcAddr = vkGetDeviceProcAddr,
		.vkCreateImage = vkCreateImage
	};
	VmaAllocatorCreateInfo allocatorCI{
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = vulkanContext.physicalDevice,
		.device = vulkanContext.device,
		.pVulkanFunctions = &vkFunctions,
		.instance = vulkanContext.instance
	};
	chk(vmaCreateAllocator(&allocatorCI, &vulkanContext.allocator));


	//Open SDL Window
	//Create a window and surface to draw to
	vulkanContext.window = SDL_CreateWindow(name.c_str(), 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(vulkanContext.window);
	chk(SDL_Vulkan_CreateSurface(vulkanContext.window, vulkanContext.instance, nullptr, &vulkanContext.surface));
	chk(SDL_GetWindowSize(vulkanContext.window, &vulkanContext.windowSize.x, &vulkanContext.windowSize.y));
	chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanContext.physicalDevice, vulkanContext.surface, &vulkanContext.surfaceCapabilities));

	//Global swapchain?
	//TODO: This is the bare minimum, and should be extended later
	/*VkExtent2D swapchainExtent{ surfaceCaps.currentExtent };
	if(surfaceCaps.currentExtent.width = 0xFFFFFFFF){
		swapchainExtent = {.width = static_cast<uint32_t>(windowSize.x), .height = static_cast<uint32_t>(windowSize.y)};
	}

	//This format is guarenteed to be supported by a graphics queue, but
	//we can get a better one if supported
	//This should probably be in a new function, as we will need to rebuild eventually.
	renderingContext.swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkSwapchainCreateInfoKHR swapchainCI{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = vulkanContext.surface,
		.minImageCount = surfaceCaps.minImageCount,
		.imageFormat = renderingContext.swapchainFormat,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent{ .width = swapchainExtent.width, .height = swapchainExtent.height },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};
	chk(vkCreateSwapchainKHR(vulkanContext.device, &swapchainCI, nullptr, &renderingContext.swapchain));
	uint32_t renderingContext.swapchainImageCount{ 0 };
	chk(vkGetSwapchainImagesKHR(vulkanContext.device, renderingContext.swapchain, &renderingContext.swapchainImageCount, nullptr));
	swapchainImages.resize(renderingContext.ImageCount);
	chk(vkGetSwapchainImagesKHR(vulkanContext.device, renderingContext.swapchain, &renderingContext.swapchainImageCount, renderingContext.swapchainImages.data()));
	swapchainImageViews.resize(renderingContext.ImageCount);
	for(auto i = 0; i < renderingContext.swapchainImageCount; i++){
		VkImageViewCreateInfo viewCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = renderingContext.swapchainImages[i],
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = renderingContext.swapchainFormat,
		.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
		};
		chk(vkCreateImageView(vulkanContext.device, &viewCI, nullptr, &redneringContext.swapchainImageViews[i]));
	}

	//Create depth attatchment
	std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
	for(VkFormat& format : depthFormatList){
		VkFormatProperties2 formatProperties = { .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
		vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &formatProperties);
		if(formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
			depthFormat = format;
			break;
		}
	}
	VkImageCreateInfo depthImageCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depthFormat,
		.extent{.width = static_cast<uint32_t>(windowSize.x), .height = static_cast<uint32_t>(windowSize.y), .depth = 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VmaAllocationCreateInfo allocCI{
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};
	chk(vmaCreateImage(allocator, &depthImageCI, &allocCI, &depthImage, &depthImageAllocation, nullptr));
	VkImageViewCreateInfo depthViewCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = depthFormat,
		.subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
	};
	chk(vkCreateImageView(device, &depthViewCI, nullptr, &depthImageView));*/

	rebuildSwapchain();
}


void Application::rebuildSwapchain(){
	chk(vkDeviceWaitIdle(vulkanContext.device));
	//Check if images and views are valid. Destroy if so.
	if(renderingContext.depthImageView != VK_NULL_HANDLE) vkDestroyImageView(vulkanContext.device, renderingContext.depthImageView, nullptr);
	if(renderingContext.depthImage != VK_NULL_HANDLE) vkDestroyImage(vulkanContext.device, renderingContext.depthImage, nullptr);
	for(uint32_t i = 0; i < renderingContext.swapchainImageCount; i++){
		if(renderingContext.swapchainImageViews[i] != VK_NULL_HANDLE){
			vkDestroyImageView(vulkanContext.device, renderingContext.swapchainImageViews[i], nullptr);
		}
	}

	//This format is guarenteed to be supported by a graphics queue, but
	//we can get a better one if supported
	//This should probably be in a new function, as we will need to rebuild eventually.
	//Swapchain creation
	//TODO: This is the bare minimum, and should be extended later
	renderingContext.swapchainExtent = vulkanContext.surfaceCapabilities.currentExtent;
	if(vulkanContext.surfaceCapabilities.currentExtent.width == 0xFFFFFFFF){
		renderingContext.swapchainExtent = {.width = static_cast<uint32_t>(vulkanContext.windowSize.x), .height = static_cast<uint32_t>(vulkanContext.windowSize.y)};
	}

	renderingContext.swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkSwapchainCreateInfoKHR swapchainCI{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = vulkanContext.surface,
		.minImageCount = vulkanContext.surfaceCapabilities.minImageCount,
		.imageFormat = renderingContext.swapchainFormat,
		.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		.imageExtent{ .width = renderingContext.swapchainExtent.width, .height = renderingContext.swapchainExtent.height },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};
	if(renderingContext.swapchain != VK_NULL_HANDLE){
		swapchainCI.oldSwapchain = renderingContext.swapchain;
		vkDestroySwapchainKHR(vulkanContext.device, renderingContext.swapchain, nullptr);
	}
	chk(vkCreateSwapchainKHR(vulkanContext.device, &swapchainCI, nullptr, &renderingContext.swapchain));
	renderingContext.swapchainImageCount = 0;
	chk(vkGetSwapchainImagesKHR(vulkanContext.device, renderingContext.swapchain, &renderingContext.swapchainImageCount, nullptr));
	renderingContext.swapchainImages.resize(renderingContext.swapchainImageCount);
	chk(vkGetSwapchainImagesKHR(vulkanContext.device, renderingContext.swapchain, &renderingContext.swapchainImageCount, renderingContext.swapchainImages.data()));
	renderingContext.swapchainImageViews.resize(renderingContext.swapchainImageCount);
	for(auto i = 0; i < renderingContext.swapchainImageCount; i++){
		VkImageViewCreateInfo viewCI = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = renderingContext.swapchainImages[i],
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = renderingContext.swapchainFormat,
		.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
		};
		chk(vkCreateImageView(vulkanContext.device, &viewCI, nullptr, &renderingContext.swapchainImageViews[i]));
	}

	//Create depth attatchment
	std::vector<VkFormat> depthFormatList{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	//VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
	for(VkFormat& format : depthFormatList){
		VkFormatProperties2 formatProperties = { .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
		vkGetPhysicalDeviceFormatProperties2(vulkanContext.physicalDevice, format, &formatProperties);
		if(formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
			renderingContext.depthFormat = format;
			break;
		}
	}
	VkImageCreateInfo depthImageCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = renderingContext.depthFormat,
		.extent{.width = static_cast<uint32_t>(vulkanContext.windowSize.x), .height = static_cast<uint32_t>(vulkanContext.windowSize.y), .depth = 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VmaAllocationCreateInfo allocCI{
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};
	chk(vmaCreateImage(vulkanContext.allocator, &depthImageCI, &allocCI, &renderingContext.depthImage, &renderingContext.depthImageAllocation, nullptr));
	VkImageViewCreateInfo depthViewCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = renderingContext.depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = renderingContext.depthFormat,
		.subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1}
	};
	chk(vkCreateImageView(vulkanContext.device, &depthViewCI, nullptr, &renderingContext.depthImageView));
}