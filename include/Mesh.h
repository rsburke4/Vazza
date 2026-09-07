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

//XMacro Magic
#define GLTF_COMPONENT_TYPES(X) \
    X(int8_t,    BYTE)           \
    X(uint8_t,   UNSIGNED_BYTE)  \
    X(int16_t,   SHORT)          \
    X(uint16_t,  UNSIGNED_SHORT) \
    X(uint32_t,  UNSIGNED_INT)   \
    X(float,     FLOAT)

template<typename T>
struct _TG3_Vec2
{
    T x, y;
};

template<typename T>
struct _TG3_Vec3
{
    T x, y, z;
};

template<typename T>
struct _TG3_Vec4
{
    T x, y, z, w;
};

template<typename T>
struct _TG3_Mat2
{
    T data[4];
};

template<typename T>
struct _TG3_Mat3
{
    T data[9];
};

template<typename T>
struct _TG3_Mat4
{
    T data[16];
};

#define MAKE_GLTF_TYPES(type, name) \
    using name##Vec2 = _TG3_Vec2<type>;  \
    using name##Vec3 = _TG3_Vec3<type>;  \
    using name##Vec4 = _TG3_Vec4<type>;  \
    using name##Mat2 = _TG3_Mat2<type>;  \
    using name##Mat3 = _TG3_Mat3<type>;  \
    using name##Mat4 = _TG3_Mat4<type>;

GLTF_COMPONENT_TYPES(MAKE_GLTF_TYPES)

#define GLTF_COMPONENT_SIZE(type, name) \
case TG3_COMPONENT_TYPE_##name: \
    return sizeof(type);

#undef MAKE_GLTF_TYPES

#define GLTF_ACCESSOR_TYPES(X) \
    X(SCALAR, 1)               \
    X(VEC2,   2)               \
    X(VEC3,   3)               \
    X(VEC4,   4)               \
    X(MAT2,   4)               \
    X(MAT3,   9)               \
    X(MAT4,   16)


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
      void Render(){};

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