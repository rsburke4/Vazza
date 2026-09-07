#include "Mesh.h"
#include "tiny_gltf_v3.h"
#include <string>

//TODO: Add assertions and error checking
template<typename T>
bool accessBuffer(const tg3_accessor &accessor, tg3_model &model, std::vector<T> &data){
    tg3_buffer_view buffer_view = model.buffer_views[accessor.buffer_view];
    uint32_t comp_type = accessor.component_type;
    uint32_t type = accessor.type; //This should be used after the data is returned
    uint64_t count = accessor.count;
    uint64_t stride = buffer_view.byte_stride;
    uint32_t size = 0;
    uint64_t offset = buffer_view.byte_offset + accessor.byte_offset;

    switch(accessor.component_type){
        default:
            return false;
        case TG3_COMPONENT_TYPE_BYTE:
            size = sizeof(int8_t);
            break;
        case TG3_COMPONENT_TYPE_DOUBLE:
            size = sizeof(double);
            break;
        case TG3_COMPONENT_TYPE_FLOAT:
            size = sizeof(float);
            break;
        case TG3_COMPONENT_TYPE_INT:
            size = sizeof(int32_t);
            break;
        case TG3_COMPONENT_TYPE_SHORT:
            size = sizeof(int16_t);
            break;
        case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
            size = sizeof(uint8_t);
            break;
        case TG3_COMPONENT_TYPE_UNSIGNED_INT:
            size = sizeof(uint32_t);
            break;
        case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
            size = sizeof(uint16_t);
            break;
    }
    switch(accessor.type){
        case TG3_TYPE_VEC2:
            size *= 2;
            break;
        case TG3_TYPE_VEC3:
            size *= 3;
            break;
        case TG3_TYPE_VEC4:
            size *= 4;
            break;
        case TG3_TYPE_MAT2:
            size *= 4;
            break;
        case TG3_TYPE_MAT3:
            size *= 9;
            break;
        case TG3_TYPE_MAT4:
            size *= 16;
            break;
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

//TODO: Overhaul mesh support with full GLTF support
//This would include a GLTF parser class for loading texture, and model resources
//Somehow Resources would need a major rework
bool Mesh::LoadMeshData(std::filesystem::path filePath, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices){
    tg3_parse_options opts;
    tg3_error_stack errors;
    tg3_model model;
    std::vector<glm::vec3> vertexBuffer;
    std::vector<uint32_t> indexBuffer;

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
        for(uint32_t i = 0; i < model.meshes[0].primitives_count; i++){
            if(model.meshes[0].primitives[i].mode == -1){
                    uint32_t vertex_i = -1;
                    uint32_t normal_i = -1;
                    uint32_t uv_i = -1;
                    for(uint32_t j = 0; j < model.meshes[0].primitives[i].attributes_count; j++){
                        const char *attrName = model.meshes[0].primitives[i].attributes[j].key.data;
                        uint32_t nameLen = model.meshes[0].primitives[i].attributes[j].key.len;
                        std::string attrString(attrName, nameLen);
                        if(attrString == "POSITION" ){
                            vertex_i = model.meshes[0].primitives[i].attributes[j].value;
                            if(model.accessors[vertex_i].type == TG3_TYPE_VEC3){
                                std::vector<glm::vec3> vertexByteBuffer;
                                accessBuffer(model.accessors[vertex_i], model, vertexByteBuffer);
                            }
                            if(model.accessors[vertex_i].type == TG3_TYPE_VEC2){

                            }
                        }
                        if(attrString == "NORMAL") normal_i = model.meshes[0].primitives[i].attributes[j].value;
                        if(attrString == "TEXCOORD_0") uv_i = model.meshes[0].primitives[i].attributes[j].value;
                        //Continue parsing primitives here
                    }
                }
        }
            //vertices.resize(model.meshes[0].);
            //vertices.resize();

    }


    tg3_model_free(&model);
    tg3_error_stack_free(&errors);
    return false;
}

void Mesh::CreateVertexBuffer(std::vector<Vertex> &vertices){
    return;
}

void Mesh::CreateIndexBuffer(std::vector<uint32_t> &indices){
    return;
}