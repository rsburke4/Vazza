#include "Mesh.h"

bool Mesh::doLoad(){
        // Step 2a: Construct file path using standardized naming convention
        std::string filePath = "models/" + GetId() + ".gltf";

        // Step 2b: Parse geometric data from file format into CPU-accessible structures
        std::vector<Vertex> vertices;      // Temporary CPU storage for vertex attributes
        std::vector<uint32_t> indices;     // Temporary CPU storage for triangle indices
        if (!LoadMeshData(filePath, vertices, indices)) {
            return false;                   // Failed to parse file - abort loading
        }

        // Step 2c: Transform CPU data into optimized GPU buffer resources
        CreateVertexBuffer(vertices);       // Upload vertex attributes to GPU
        CreateIndexBuffer(indices);         // Upload triangle connectivity to GPU

        // Step 2d: Cache metadata for efficient rendering operations
        vertexCount = static_cast<uint32_t>(vertices.size());
        indexCount = static_cast<uint32_t>(indices.size());

        return Resource::Load();            // Mark resource as successfully loaded
}

bool Mesh::doUnload(){
    // Only proceed with cleanup if resources are currently loaded
    if (IsLoaded()) {
        // Phase 3a: Obtain device handle for resource destruction
        VkDevice device = Application::GetInstance()->GetVulkanContext()->device;

        //This is the general structure for unloading but it's not the right syntax
       /* // Phase 3b: Destroy buffers and free GPU memory in proper sequence
        // Index resources cleaned up first to maintain clear dependency order
        device.destroyBuffer(indexBuffer);         // Destroy index buffer object
        device.freeMemory(indexBufferMemory);      // Release index buffer memory

        // Vertex resources cleaned up second
        device.destroyBuffer(vertexBuffer);        // Destroy vertex buffer object
        device.freeMemory(vertexBufferMemory);     // Release vertex buffer memory

        // Phase 3c: Update base class state to reflect unloaded condition
        Resource::Unload();*/
        return true;
    }
    return false;
}

bool Mesh::LoadMeshData(std::filesystem::path filePath, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices){

}

void Mesh::CreateVertexBuffer(std::vector<Vertex> &vertices){

}

void Mesh::CreateIndexBuffer(std::vector<uint32_t> &indices){

}