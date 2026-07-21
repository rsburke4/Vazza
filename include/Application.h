#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <string>
#include <iostream>

struct VulkanContext{
    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

struct RenderingContext{
    const uint32_t maxFramesInFlight{ 2 };
    uint32_t imageIndex{ 0 };
    uint32_t frameIndex{ 0 };
    bool updateSwapchain{ false };
};

class Application
{
  public:
    void InitializeVulkan();
    void DrawFrame();


    void chk(VkResult result)
    {
        if (result != VK_SUCCESS)
        {
            std::cerr << "Vulkan call returned an error (" << result << ")\n";
            exit(result);
        }
    }

    void chk(bool result)
    {
        if (!result)
        {
            std::cerr << "Call returned an error\n";
            exit(result);
        }
    }

    void chkSwapchain(VkResult result)
    {
        if (result < VK_SUCCESS)
        {
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                renderingContext.updateSwapchain = true;
                return;
            }
            std::cerr << "Vulkan swapchain call returned an error (" << result << ")\n";
            exit(result);
        }
    }

    const VulkanContext* GetVulkanContext() const{
        return &vulkanContext;
    }

    RenderingContext* GetRenderingContext(){
        return &renderingContext;
    }

    const std::string name = "VAZZA";
    const std::string version = "0.0.0";

    private:
        VulkanContext vulkanContext;
        RenderingContext renderingContext;
};