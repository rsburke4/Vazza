#include "Texture.h"

bool Texture::doLoad(){
    //Work with all ktx internally
    std::string filePath = "textures/" + GetId() + ".ktx";

    //Load raw image data from disk with format detection
    unsigned char* data = LoadImageData(filePath, &width, &height, &channels);
    if(!data){
        return false; //Failed to load image
    }
    //Transform raw pixel data into Vulkan GPU resources
    CreateVulkanImage(data, width, height, channels);
    //Clean up temporary CPU memory to prevent leaks
    FreeImageData(data);

    return true;
}

bool Texture::doUnload(){
    //Only perform cleanup is resource is loaded
    if(IsLoaded()){
        //Get the device handle for resource destruction
        VkDevice device = GetDevice();

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

unsigned char* Texture::LoadImageData(const std::string& filePath, int* width, int* height, int* channels){
    return nullptr;
}

void Texture::FreeImageData(unsigned char* data){
    return;
}

void Texture::CreateVulkanImage(unsigned char* data, int width, int height, int channels){
    return;
}

VkDevice Texture::GetDevice(){
    VkDevice notRealDevice;
    return notRealDevice;
}
