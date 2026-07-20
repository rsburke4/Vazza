#include "Globals.h"
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
    
}