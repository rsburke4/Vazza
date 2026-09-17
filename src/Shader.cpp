#include <iostream>
#include <fstream>
#include "Shader.h"
#include "Application.h"

//Regardless of filename, shader flags determine how a file is read
bool Shader::ReadFile(std::vector<uint32_t>& buffer){
    // Open in binary mode
    std::ifstream file(filePath.string(), std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath.string() << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // SPIR-V bytecode size must be a multiple of 4 bytes
    if (size % 4 != 0) {
        std::cerr << "Shader file size is not a multiple of 4: " << size << std::endl;
        return false;
    }

    // Resize vector to hold the correct number of 32-bit words
    buffer.resize(size / 4);

    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cout << "Read shader " << filePath.string() << std::endl;
        return true;
    }
    
    std::cerr << "Error when reading in shader " << filePath.string() << std::endl;
    return false;
}

void Shader::CreateShaderModule(const std::vector<uint32_t>& buffer){
    VkDevice device = Application::GetInstance()->GetVulkanContext()->device;
	VkShaderModuleCreateInfo shaderModuleCI{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = buffer.size() * sizeof(uint32_t),
		.pCode = reinterpret_cast<const uint32_t*>(buffer.data())
	};

	chk(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule));
}

bool Shader::doLoad(){
    std::vector<uint32_t> codeBuffer;
     std::string extention = filePath.string().substr(filePath.string().length() - 9);
    if(extention == ".vert.spv"){
        stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else if(extention == ".frag.spv"){
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    else if(extention == ".comp.spv"){
        stage = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    else{
        std::cerr << "Shader " << filePath.string() << " should end in .vert.spv, .frag.spv or .comp.spv\n";
        return false;
    }
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