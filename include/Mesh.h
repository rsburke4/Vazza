#ifndef __MESH_RESOURCE_H__
#define __MESH_RESOURCE_H__

#include "Resource.h"
#include "Application.h"
#include <vulkan/vulkan.h>
#include <filesystem>

struct Vertex{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
  //glm::vec4 color;
};

class Mesh : public Resource{
    public:
        explicit Mesh(const std::string& id) : Resource(id), filePath(id) {}

      ~Mesh() override {
        Unload();                           // Ensure GPU resources are cleaned up
      }

      bool doLoad() override;
      bool doUnload() override;

      VkBuffer GetVertexBuffer() const { return vertexBuffer; }
      VkBuffer GetIndexBuffer() const { return indexBuffer; }
      uint32_t GetVertexCount() const { return vertexCount; }
      uint32_t GetIndexCount() const { return indexCount; }

    private:
      bool LoadMeshData(std::filesystem::path filePath, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
      void CreateVertexBuffer(std::vector<Vertex> &vertices);       // Upload vertex attributes to GPU
      void CreateIndexBuffer(std::vector<uint32_t> &indices);
      
      VkBuffer vertexBuffer;                // GPU buffer containing vertex attribute data
      VkDeviceMemory vertexBufferMemory;    // GPU memory backing the vertex buffer
      VkDeviceSize vertexBufferOffset;      // Offset within the memory allocation for vertex buffer
      uint32_t vertexCount = 0;               // Number of vertices in this mesh

      // Index data management - defines triangle connectivity using vertex indices
      VkBuffer indexBuffer;                 // GPU buffer containing triangle index data
      VkDeviceMemory indexBufferMemory;     // GPU memory backing the index buffer
      VkDeviceSize indexBufferOffset;       // Offset within the memory allocation for index buffer
      uint32_t indexCount = 0;                // Number of indices in this mesh (typically 3 per triangle)

      const std::filesystem::path filePath;
};

#endif