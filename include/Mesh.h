#define GLTF_COMPONENT_TYPES(X) \
    X(int8_t,    BYTE)           \
    X(uint8_t,   UNSIGNED_BYTE)  \
    X(int16_t,   SHORT)          \
    X(uint16_t,  UNSIGNED_SHORT) \
    X(uint32_t,  UNSIGNED_INT)   \
    X(float,     FLOAT)

#define GLTF_ACCESSOR_TYPES(X) \
    X(SCALAR, 1)               \
    X(VEC2,   2)               \
    X(VEC3,   3)               \
    X(VEC4,   4)               \
    X(MAT2,   4)               \
    X(MAT3,   9)               \
    X(MAT4,   16)


#ifndef __MESH_RESOURCE_H__
#define __MESH_RESOURCE_H__
#include <cstdint>
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

//XMacro Magic
#define X(type, name) \
  struct name##_TG3_Vec2 { type x, y; }; \
  struct name##_TG3_Vec3 { type x, y, z; }; \
  struct name##_TG3_Vec4 { type x, y, z, w; }; \
  struct name##_TG3_Mat2 { type data[4]; }; \
  struct name##_TG3_Mat3 { type data[9]; }; \
  struct name##_TG3_Mat4 { type data[16]; };

GLTF_COMPONENT_TYPES(X)
#undef X


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
            bool LoadMeshData(std::filesystem::path filePath,
              std::vector<Vertex> &vertices,
              std::vector<uint32_t> &indices);
      void Render(){};

    private:

      void CreateVertexBuffer(std::vector<Vertex> &vertices);       // Upload vertex attributes to GPU
      void CreateIndexBuffer(std::vector<uint32_t> &indices);
      
      VkBuffer vertexBuffer;                // GPU buffer containing vertex attribute data
      VmaAllocation vBufferAllocation;
      VkDeviceSize vertexBufferOffset;      // Offset within the memory allocation for vertex buffer
      uint32_t vertexCount = 0;               // Number of vertices in this mesh

      // Index data management - defines triangle connectivity using vertex indices
      VkBuffer indexBuffer;                 // GPU buffer containing triangle index data
      VmaAllocation iBufferAllocation;
      VkDeviceSize indexBufferOffset;       // Offset within the memory allocation for index buffer
      uint32_t indexCount = 0;                // Number of indices in this mesh (typically 3 per triangle)

      const std::filesystem::path filePath;
};

#endif