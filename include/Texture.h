#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "Resource.h"
#include "Application.h"
#include <vulkan/vulkan.h>
#include <filesystem>

class Texture : public Resource{
    //All the good stuff for loading images
    private:
        VkImage image;          //GPU image object containing pixel data
        VkDeviceMemory memory;  //GPU memory allocation backing the image
        VkDeviceSize offset;    //Offset within the memory allocation for this texture
        VkImageView imageView;  //Shader-accessible view into the image
        VkSampler sampler;      //Sampling configuration
        VkFormat format;        //Format of the image
        VmaAllocation allocation;
        std::filesystem::path filePath;


    //All the normal information about the image
        int width = 0;
        int height = 0;
        int layers = 0; //Will usually be 1?
        int levels = 0;

        unsigned char* LoadImageData(uint32_t &size);
        void FreeImageData(unsigned char* data);
        void CreateVulkanImage(unsigned char* data, uint32_t size);

    public:
        explicit Texture(const std::string& id, std::filesystem::path filename) : Resource(id) {}

        ~Texture() override{
            Unload();
        }

        bool doLoad() override;
        bool doUnload() override;

        VkImage GetImage() const {return image;}
        VkImageView GetImageView() const { return imageView;}
        VkSampler GetSampler() const { return sampler;} 
};

#endif