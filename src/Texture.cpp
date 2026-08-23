#include "Application.h"
#include "volk.h"
#include "Texture.h"
#include "ktx.h"
#include "ktxvulkan.h"
#include <stdexcept>
#include <cstring>
#include <vulkan/vulkan.h>
#include <filesystem>


bool Texture::doLoad(){
    //Work with all ktx internally

    //Load raw image data from disk with format detection
    uint32_t size;
    unsigned char* data = LoadImageData(&size);
    if(!data){
        return false; //Failed to load image
    }
    //Transform raw pixel data into Vulkan GPU resources
    CreateVulkanImage(data, size);
    //Clean up temporary CPU memory to prevent leaks
    FreeImageData(data);

    return true;
}

bool Texture::doUnload(){
    //Only perform cleanup is resource is loaded
    if(IsLoaded()){
        //Get the device handle for resource destruction
        VkDevice device = Application::GetInstance()->GetVulkanContext()->device;

        //Destroy GPU objects in reverse creation order
        //This ordering prevents use-after-free errors in GPU drivers
        vkDestroySampler(device, sampler, VK_NULL_HANDLE);
        vkDestroyImageView(device, imageView, VK_NULL_HANDLE);
        vkDestroyImage(device, image, VK_NULL_HANDLE);
        vkFreeMemory(device, memory, VK_NULL_HANDLE);

        return true;
    }
    return false;
}

//TODO: Return format type as well?
unsigned char* Texture::LoadImageData(uint32_t &size){
    ktxTexture* tempTexture{nullptr};
    KTX_error_code err = ktxTexture_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &tempTexture);
    if(err != KTX_SUCCESS){
        //TODO: Replace with cached "missing texture image"
        throw std::runtime_error("Problem loading image from disk");
        return nullptr;
    }

    //Check to see if KTX2 is used. Convert as needed
    if(tempTexture->classId == ktxTexture2_c){
        ktxTexture2* ktx2 = reinterpret_cast<ktxTexture2*>(tempTexture);
        KTX_error_code transcodeRes = ktxTexture2_TranscodeBasis(ktx2, KTX_TTF_BC7_RGBA, 0);
        if(transcodeRes != KTX_SUCCESS){
            ktxTexture_Destroy(tempTexture);
            throw std::runtime_error("Problem transcribing KTX2");
            return nullptr;
        }
    }

    //Fetch size and internal pointer
    size_t bufferSize = ktxTexture_GetDataSize(tempTexture);
    ktx_uint8_t* internalPtr = ktxTexture_GetData(tempTexture);
    width = tempTexture->baseWidth;
    height = tempTexture->baseHeight;
    levels = tempTexture->numLevels;
    layers = tempTexture->numLayers;
    format = ktxTexture_GetVkFormat(tempTexture);
 
    //Make persistent buffer and return
    unsigned char* externalBuffer = new unsigned char[bufferSize];
    size = bufferSize;
    std::memcpy(externalBuffer, internalPtr, bufferSize);
    ktxTexture_Destroy(tempTexture);

    return externalBuffer;
}

void Texture::FreeImageData(unsigned char* data){
    delete[] data;
}

void Texture::CreateVulkanImage(unsigned char* data, uint32_t size){
        // Implementation to create Vulkan image, allocate memory, and upload data
        Application *appInstance = Application::GetInstance();
        VkDevice device = appInstance->GetVulkanContext()->device;

        //Create destination image
        VkImageCreateInfo imageCI = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {.width = width, .height = height, .depth = 1},
            .mipLevels = levels,
            .arrayLayers = layers, //Maybe? 1?
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        //Memory allocation
		VmaAllocationCreateInfo imageAllocCI{.usage = VMA_MEMORY_USAGE_AUTO };
		chk(vmaCreateImage(appInstance->GetVulkanContext()->allocator, &imageCI, &imageAllocCI, &image, &allocation, nullptr));

        // - Data upload via staging buffers for efficiency
        // - Image view creation for shader access
        VkImageViewCreateInfo imageViewCI = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = levels, .layerCount = layers}
        };
        chk(vkCreateImageView(device, &imageViewCI, nullptr, &imageView));
        // - Sampler creation with appropriate filtering settings
        //This should be something we can set later on.
        VkSamplerCreateInfo samplerCI{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 8.0f, //Widely supported constant
			.maxLod = (float)levels
        };
        chk(vkCreateSampler(device, &samplerCI, nullptr, &sampler));

        //Create command buffer to move image data to VkImage on GPU
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer = {};
        VkCommandPoolCreateInfo commandPoolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = appInstance->GetVulkanContext()->graphicsQueueFamily
        };
        chk(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool));
        VkCommandBufferAllocateInfo commandBufferAI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        chk(vkAllocateCommandBuffers(device, &commandBufferAI, &commandBuffer));
        VkCommandBufferBeginInfo commandBufferBI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        chk(vkBeginCommandBuffer(commandBuffer, &commandBufferBI));

        //Set up memory barrier to get data into optimal transfer format
        VkImageMemoryBarrier2KHR imgWriteBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR,
			.srcStageMask = VK_PIPELINE_STAGE_2_NONE,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.image = image,
			.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = levels, .layerCount = layers}
		};
        VkDependencyInfo imgWriteDI{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imgWriteBarrier
        };
        vkCmdPipelineBarrier2KHR(commandBuffer, &imgWriteDI);

        //Copy the actual buffer
        VkBuffer imageBuffer;
        VkBufferCreateInfo imageBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .size = size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,

        };
        VmaAllocationCreateInfo imgSrcAllocCI{
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO
		};
        VmaAllocation imgSrcAllocation {};
        VmaAllocationInfo imgSrcAllocInfo {};
        chk(vmaCreateBuffer(appInstance->GetVulkanContext()->allocator, &imageBufferCI, &imgSrcAllocCI, &imageBuffer, &imgSrcAllocation, &imgSrcAllocInfo));
        //Put data into host-side VkBuffer
        memcpy(imgSrcAllocInfo.pMappedData, data, size);

        vkCmdCopyBufferToImage(commandBuffer, imageBuffer, image, layout, regionCount, pRegions);
        chk(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo{
             
        };
        chk(vkQueueSubmit(appInstance->GetVulkanContext()->graphicsQueue, ));
    return;
}