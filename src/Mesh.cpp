//#include "../include/Mesh.h"


#include "Mesh.h"
#include "tiny_gltf_v3.h"
#include <string>


//TODO: Add assertions and error checking
template<typename T>
bool accessBuffer(const tg3_accessor &accessor, tg3_model &model, std::vector<T> &data){
    tg3_buffer_view buffer_view = model.buffer_views[accessor.buffer_view];
    uint32_t comp_type = accessor.component_type;
    uint32_t acc_type = accessor.type; //This should be used after the data is returned
    uint64_t count = accessor.count;
    uint64_t stride = buffer_view.byte_stride;
    uint32_t size = 0;
    uint64_t offset = buffer_view.byte_offset + accessor.byte_offset;

    switch(comp_type){
        #define X(type, name) \
        case TG3_COMPONENT_TYPE_##name: \
            size = sizeof(type); \
            break; 
        GLTF_COMPONENT_TYPES(X)
        #undef X
    }
    switch(acc_type){
        #define X(type, num) \
        case(TG3_TYPE_##type): \
            size *= num; \
            break; 
        GLTF_ACCESSOR_TYPES(X)
        #undef X
    }

    
    if(stride == 0) stride = size;
    if(sizeof(T) != size)
    {
        std::cerr << "Accessor size does not match destination type\n";
        return false;
    }

    //If sparse
    uint32_t sparce = accessor.sparse.is_sparse;
    uint32_t sparce_count = accessor.sparse.count;
    tg3_accessor_sparse_indices sparce_incides;
    tg3_accessor_sparse_values sparse_values;


    if(accessor.normalized == 1){
        std::cerr << "Normalized buffer not yet supported\n";
            return false;
    }

    for(uint64_t i = 0; i < count; i++){
        T dataVal;
        memcpy(&dataVal, model.buffers[buffer_view.buffer].data.data + offset + i * stride, size);
        data.push_back(dataVal);
    }
    return true;
}

bool Mesh::doLoad(){
        // Step 2a: Construct file path using standardized naming convention
        std::string filePath = "models/" + GetId() + ".gltf";

        // Step 2b: Parse geometric data from file format into CPU-accessible structures
        std::vector<Vertex> vertices;      // Temporary CPU storage for vertex attributes
        std::vector<uint32_t> indices;     // Temporary CPU storage for triangle indices
        std::vector<Vertex> normals;
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

//TODO: These are very similar. Maybe a helper function for creating buffers?
void Mesh::CreateVertexBuffer(std::vector<Vertex> &vertices){
    VkDeviceSize vBufferSize {sizeof(Vertex) * vertices.size()};
    VkBufferCreateInfo vBufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = vBufferSize,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };
    VmaAllocationCreateInfo vBufferAllocCI{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
	};
    VmaAllocationInfo vBufferAllocInfo{};
    VmaAllocator allocator = Application::GetInstance()->GetVulkanContext()->allocator;
    chk(vmaCreateBuffer(allocator, &vBufferCI, &vBufferAllocCI, &vertexBuffer, &vBufferAllocation, &vBufferAllocInfo));
    memcpy(vBufferAllocInfo.pMappedData, vertices.data(), vBufferSize);
}

void Mesh::CreateIndexBuffer(std::vector<uint32_t> &indices){
    VkDeviceSize iBufferSize {sizeof(uint32_t) * indices.size()};
    VkBufferCreateInfo iBufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = iBufferSize,
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };
    VmaAllocationCreateInfo iBufferAllocCI{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO
    };
    VmaAllocationInfo iBufferAllocInfo{};
    VmaAllocator allocator = Application::GetInstance()->GetVulkanContext()->allocator;
    chk(vmaCreateBuffer(allocator, &iBufferCI, &iBufferAllocCI, &vertexBuffer, &iBufferAllocation, &iBufferAllocInfo));
    memcpy(iBufferAllocInfo.pMappedData, indices.data(), iBufferSize);
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

//TODO: Overhaul mesh support with full GLTF support
//This would include a GLTF parser class for loading texture, and model resources
//Somehow Resources would need a major rework
bool Mesh::LoadMeshData(std::filesystem::path filePath,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices){
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model model;
    std::vector<glm::vec3> vertexBuffer;
    std::vector<glm::vec3> normalBuffer;
    std::vector<uint32_t> indexBuffer;
    std::vector<glm::vec2> uvBuffer;
    
    tg3_parse_options_init(&opts);
    tg3_error_stack_init(&errors);
    
    tg3_error_code err = tg3_parse_file(&model, &errors, filePath.c_str(), filePath.string().length(), &opts);
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count; i++) {
            fprintf(stderr, "[%d] %s\n", (int)errors.entries[i].severity,
            errors.entries[i].message ? errors.entries[i].message : "(null)");
        }
    }
    
    // ... use model ...
    if(model.meshes_count > 0){
        //Load mesh
        
        //TODO: There's a lot of wasted memory in here
        //TODO: Put a lot of this in a function
        for(uint32_t i = 0; i < model.meshes[0].primitives_count; i++){
            if(model.meshes[0].primitives[i].mode == TG3_MODE_TRIANGLES){
                uint32_t vertex_i = -1;
                uint32_t normal_i = -1;
                uint32_t uv_i = -1;
                uint32_t indices_i = model.meshes[0].primitives->indices;
                //Only one index array per primitive
                if(indices_i >= 0){
                    //Load indices here
                    if(model.accessors[indices_i].type == TG3_TYPE_SCALAR){
                        switch(model.accessors[indices_i].component_type){
                            #define X(type, name) \
                            case TG3_COMPONENT_TYPE_##name: { \
                                std::vector<type> tempBuffer; \
                                accessBuffer(model.accessors[indices_i], model, tempBuffer); \
                                for(uint64_t i_i = 0; i_i < model.accessors[indices_i].count; i_i++){ \
                                    indexBuffer.push_back(static_cast<uint32_t>(tempBuffer[i_i])); \
                                } \
                                break; \
                            }
                            GLTF_COMPONENT_TYPES(X)
                            #undef X
                        }
                    }
                    else{
                        continue;
                    }
                }
                //Possibly multiple attributes per mesh
                for(uint32_t j = 0; j < model.meshes[0].primitives[i].attributes_count; j++){
                    const char *attrName = model.meshes[0].primitives[i].attributes[j].key.data;
                    uint32_t nameLen = model.meshes[0].primitives[i].attributes[j].key.len;
                    std::string attrString(attrName, nameLen);
                    //Could this also be a macro? Is that too much?
                    if(attrString == "POSITION"){
                        vertex_i = model.meshes[0].primitives[i].attributes[j].value;
                        //Select the appropriate struct for the component type
                        if(model.accessors[vertex_i].type == TG3_TYPE_VEC3){
                            switch (model.accessors[vertex_i].component_type){
                                #define X(type, name) \
                                case TG3_COMPONENT_TYPE_##name: { \
                                    std::vector<name##_TG3_Vec3> tempBuffer; \
                                    accessBuffer(model.accessors[vertex_i], model, tempBuffer); \
                                    for(uint64_t v_i = 0; v_i < model.accessors[vertex_i].count; v_i++){ \
                                        vertexBuffer.push_back(glm::vec3(tempBuffer[v_i].x, tempBuffer[v_i].y, tempBuffer[v_i].z)); \
                                    } \
                                    break; \
                                }
                                 GLTF_COMPONENT_TYPES(X)
                                #undef X
                            }
                        }
                    }
                    if(attrString == "NORMAL"){
                        normal_i = model.meshes[0].primitives[i].attributes[j].value;
                        if(model.accessors[normal_i].type == TG3_TYPE_VEC3){
                            switch(model.accessors[normal_i].component_type){
                                #define X(type, name) \
                                case TG3_COMPONENT_TYPE_##name: { \
                                    std::vector<name##_TG3_Vec3> tempBuffer; \
                                    accessBuffer(model.accessors[normal_i], model, tempBuffer); \
                                    for(uint64_t n_i = 0; n_i < model.accessors[normal_i].count; n_i++){ \
                                        normalBuffer.push_back(glm::vec3(tempBuffer[n_i].x, tempBuffer[n_i].y, tempBuffer[n_i].z)); \
                                    } \
                                    break; \
                                }
                                GLTF_COMPONENT_TYPES(X)
                                #undef X
                            }
                        }
                    }
                    if(attrString == "TEXCOORD_0"){
                        uv_i = model.meshes[0].primitives[i].attributes[j].value;
                        if(model.accessors[uv_i].type == TG3_TYPE_VEC2){
                            switch(model.accessors[uv_i].component_type){
                                #define X(type, name) \
                                case TG3_COMPONENT_TYPE_##name: { \
                                    std::vector<name##_TG3_Vec2> tempBuffer; \
                                    accessBuffer(model.accessors[uv_i], model, tempBuffer); \
                                    for(uint64_t t_i = 0; t_i < model.accessors[uv_i].count; t_i++){ \
                                        uvBuffer.push_back(glm::vec2(tempBuffer[t_i].x, tempBuffer[t_i].y)); \
                                    } \
                                    break; \
                                }
                                    GLTF_COMPONENT_TYPES(X)
                                    #undef X
                            }
                        }
                    }
                }
                if(vertex_i != -1){
                    vertexCount = static_cast<uint32_t>(model.accessors[vertex_i].count);
                    vertices.resize(vertexCount);
                    for(int32_t verts = 0; verts < model.accessors[vertex_i].count; verts++){
                        vertices[verts].pos = glm::vec3(vertexBuffer[verts].x, vertexBuffer[verts].y, vertexBuffer[verts].z);
                    }
                }
                if(normal_i != -1){
                    for(int32_t verts = 0; verts < model.accessors[vertex_i].count; verts++){
                        vertices[verts].normal = glm::vec3(normalBuffer[verts].x, normalBuffer[verts].y, normalBuffer[verts].z);
                    }          
                }
                if(uv_i != -1){
                    for(int32_t verts = 0; verts < model.accessors[vertex_i].count; verts++){
                        vertices[verts].uv = glm::vec2(uvBuffer[verts].x, uvBuffer[verts].y);
                    }
                }
            }
            else{
                continue;
            }
        }
    }
    if(vertices.size() == 0){
        std::cerr << "Failed to load vertex buffer\n";
        return false;
    }

    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
    return true;
}