#include "Texture.h"
#include "ktx.h"
#include "ktxvulkan.h"
#include <stdexcept>
#include <cstring>

bool Texture::doLoad(){
    //Work with all ktx internally
    std::string filePath = "textures/" + GetId() + ".ktx";

    //Load raw image data from disk with format detection
    unsigned int imgFormat = 0; //Should we save this internally?
    unsigned char* data = LoadImageData(filePath, &width, &height, &channels, &imgFormat);
    if(!data){
        return false; //Failed to load image
    }
    //Transform raw pixel data into Vulkan GPU resources
    CreateVulkanImage(data, width, height, channels, imgFormat);
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

//TODO: Return format type as well?
unsigned char* Texture::LoadImageData(const std::string& filePath, int* width, int* height, int* channels, unsigned int* outVkFormat){
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
    if(width) *width = tempTexture->baseWidth;
    if(height) *height = tempTexture->baseHeight;
    if(channels) *channels = 4; // <- Lets hope
    if(outVkFormat) *outVkFormat = ktxTexture_GetVkFormat(tempTexture);
 
    //Make persistent buffer and return
    unsigned char* externalBuffer = new unsigned char[bufferSize];
    std::memcpy(externalBuffer, internalPtr, bufferSize);
    ktxTexture_Destroy(tempTexture);

    return externalBuffer;
}

void Texture::FreeImageData(unsigned char* data){
    delete[] data;
}

void Texture::CreateVulkanImage(unsigned char* data, int width, int height, int channels, unsigned int inVkFormat){
    
    return;
}

VkDevice Texture::GetDevice(){
    VkDevice notRealDevice;
    return notRealDevice;
}
