#include "Application.h"
#include "volk.h"
#include "Texture.h"
#include "ktx.h"
#include "ktxvulkan.h"
#include <stdexcept>
#include <cstring>
#include <vulkan/vulkan.h>

bool Texture::doLoad(){
    //Work with all ktx internally
    std::string filePath = "textures/" + GetId() + ".ktx";

    //Load raw image data from disk with format detection
    unsigned int imgFormat = 0; //Should we save this internally?
    unsigned char* data = LoadImageData(filePath);
    if(!data){
        return false; //Failed to load image
    }
    //Transform raw pixel data into Vulkan GPU resources
    CreateVulkanImage(data, width, height, channels, levels, layers, imgFormat);
    //Clean up temporary CPU memory to prevent leaks
    FreeImageData(data);

    return true;
}

bool Texture::doUnload(){
    //Only perform cleanup is resource is loaded
    if(IsLoaded()){
        //Get the device handle for resource destruction
        VkDevice device = Application::GetInstance->GetVulkanContext()->device;

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
unsigned char* Texture::LoadImageData(const std::string& filePath){
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
    channels = tempTexture->channels; // <- Lets hope
    levels = tempTexture->numLevels;
    layers = tempTexture->numLayers;
    format = ktxTexture_GetVkFormat(tempTexture);
 
    //Make persistent buffer and return
    unsigned char* externalBuffer = new unsigned char[bufferSize];
    std::memcpy(externalBuffer, internalPtr, bufferSize);
    ktxTexture_Destroy(tempTexture);

    return externalBuffer;
}

void Texture::FreeImageData(unsigned char* data){
    delete[] data;
}

void Texture::CreateVulkanImage(unsigned char* data){
        // Implementation to create Vulkan image, allocate memory, and upload data
        Application *appInstance = Application::GetInstance();

        //Create destination image
        VkImageCreateInfo imageCI = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .imageType = format,
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
            .format = inVkFormat,
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = levels, .layerCount = layers}
        };
        chk(vkCreateImageView(appInstance->GetVulkanContext()->device, &imageViewCI, nullptr, &imageView));
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
        chk(vkCreateSampler(appInstance->GetVulkanContext()->device, &samplerCI, nullptr, &sampler));
    return;
}

VkDevice Texture::GetDevice(){
    VkDevice notRealDevice;
    return notRealDevice;
}
