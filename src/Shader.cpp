#include <iostream>
#include <fstream>
#include "Shader.h"
#include "Application.h"

//Regardless of filename, shader flags determine how a file is read
bool Shader::ReadFile(std::vector<char>& buffer){
    std::ifstream file(filePath.string(), std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath.string() << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    buffer.resize(size);

    if(file.read(buffer.data(), size)){
        std::cout << "Read shader " << filePath.string() << std::endl;
    }
    else{
        std::cerr << "Error when reading in shader " << filePath.string() << std::endl;
    }

    return true;
}

void Shader::CreateShaderModule(const std::vector<char>& buffer){
    VkDevice device = Application::GetInstance()->GetVulkanContext()->device;
	VkShaderModuleCreateInfo shaderModuleCI{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = (uint32_t)buffer.size(),
		.pCode = (uint32_t*)buffer.data()
	};
	VkShaderModule shaderModule{};
	chk(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));
}

bool Shader::doLoad(){
    std::vector<char> codeBuffer;
    if(!ReadFile(codeBuffer)) return false;

    CreateShaderModule(codeBuffer);

    return true;
}

bool Shader::doUnload(){
    if(IsLoaded()){
        VkDevice device = Application::GetInstance()->GetVulkanContext()->device;
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }

    return true;
}