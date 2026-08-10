#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#define VK_NO_PROTOTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define TINYOBJLOADE_IMPLEMENTATION


#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <iostream>

//Global variables for simple rendering
struct VulkanContext{
    //Instances and devices
    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};

    //Queues and commands
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    uint32_t graphicsQueueFamily = -1;
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VmaAllocator allocator{VK_NULL_HANDLE};

    //Surface Related
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    SDL_Window *window;
    glm::ivec2 windowSize{};
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_dynamic_rendering", "VK_KHR_synchronization2" };
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
};

struct RenderingContext{
    const uint32_t maxFramesInFlight{ 2 };
    uint32_t imageIndex{ 0 };
    uint32_t frameIndex{ 0 };

    //Swapchain information
    bool updateSwapchain{ false };
    uint32_t swapchainImageCount = 0;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VkExtent2D swapchainExtent;
    VkFormat swapchainFormat{VK_FORMAT_UNDEFINED};

    //Depth image information
    //Wouldn't this be better to have under a camera object?
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};
    VkImage depthImage{VK_NULL_HANDLE};
    VkImageView depthImageView{VK_NULL_HANDLE};
    VmaAllocation depthImageAllocation{VK_NULL_HANDLE};
};

class Application
{
  public:
    void InitializeVulkan();
    void DrawFrame();


    //TODO: Make static or move to global class
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

    const RenderingContext* GetRenderingContext() const{
        return &renderingContext;
    }

    void rebuildSwapchain();

    const std::string name = "VAZZA";
    const std::string version = "0.0.0";

    private:
        VulkanContext vulkanContext;
        RenderingContext renderingContext;
};

#endif