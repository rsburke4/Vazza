#ifndef __SHADER_H__
#define __SHADER_H__

#include <vulkan/vulkan.h>
#include <vector>
#include <filesystem>
#include "Resource.h"

class Shader : public Resource{
    public:
        Shader(const std::string& id, VkShaderStageFlagBits shaderStage) : Resource(id), filePath(id), stage(shaderStage){}
        ~Shader() override{
            Unload();
        }

        bool doLoad() override;
        bool doUnload() override;

        VkShaderModule GetShaderModule() const { return shaderModule; }
        VkShaderStageFlagBits GetStage() const { return stage; }

    private:
        bool ReadFile(std::vector<char>& buffer);
        void CreateShaderModule(const std::vector<char>& buffer);

        VkShaderModule shaderModule;
        VkShaderStageFlagBits stage;
        std::filesystem::path filePath;
        
};

#endif