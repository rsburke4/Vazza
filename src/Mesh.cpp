#include "Mesh.h"
#include "tiny_gltf_v3.h"
#include <string>

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
    //TODO: Add support for multiple uv sets
    if(model.meshes_count > 0){
        for(uint32_t i = 0; i < model.meshes[0].primitives_count; i++){
            if(model.meshes[0].primitives[i].mode == -1){
                    uint32_t vertex_i = -1;
                    uint32_t normal_i = -1;
                    uint32_t uv_i = -1;
                    for(uint32_t j = 0; j < model.meshes[0].primitives[i].attributes_count; j++){
                        const char *attrName = model.meshes[0].primitives[i].attributes[j].key.data;
                        uint32_t nameLen = model.meshes[0].primitives[i].attributes[j].key.len;
                        std::string attrString(attrName, nameLen);
                        if(attrString == "POSITION" ) vertex_i = model.meshes[0].primitives[i].attributes[j].value;
                        if(attrString == "NORMAL") normal_i = model.meshes[0].primitives[i].attributes[j].value;
                        if(attrString == "TEXCOORD_0") uv_i = model.meshes[0].primitives[i].attributes[j].value;
                    }
                    if(vertex_i > 0){
                       // model.accessors[vertex_i]
                    }
                    if(normal_i > 0){
                       // model.accessors[normal_i]
                    }
                    if(uv_i > 0){
                       // model.accessors[uv_i]
                    }
            }

            //vertices.resize(model.meshes[0].);
            //vertices.resize();
        }
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