#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

class ResourceManager;
template <typename T>
class ResourceHandle;

class Resource
{
  private:
    std::string resourceId;
    bool loaded = false;

  public:
    explicit Resource(const std::string &id) : resourceId(id)
    {
    }
    virtual ~Resource();

    // Core resource identity and state access methods
    const std::string &GetID() const
    {
        return resourceId;
    }
    bool IsLoaded() const
    {
        return loaded;
    }

    bool Load()
    {
        loaded = doLoad();
        return loaded;
    }

    void Unload()
    {
        doUnload();
        loaded = false;
    }

  protected:
    virtual bool doLoad() = 0;
    virtual bool doUnload() = 0;
};

class ResourceManager
{
  private:
    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;

    struct ResourceData
    {
        std::shared_ptr<Resource> resource;
        int refCount;
    };
    std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> refCounts;

    template<typename T>
    ResourceHandle<T> Load(const std::string &resourceId){
        static_assert(std::is_base_of<Resource, T>::value, "T must derive from Resource");

        //Check if existing resource cache to avoid redundant loading
        auto& typeResources = resources[std::type_index(typeid(T))];
        auto it = typeResources.find(resourceId);
        if(it != typeResources.end()){
            auto& refDataCounts = refCounts[std::type_index(typeid(T))];
            auto refIt = refDataCounts.find(resourceId);
            refIt->second.refCount++;
            return ResouceHandle<T>(resourceId, this);
        }

        //Create new resource instance and attempt loading
        auto resource = std::make_shared<T>(resourceId);
        if(!resource->Load()){
            //Load failed. Return invalid handle rather than corrupt cache.
            return ResourceHandle<T>();
        }

        //Cache successful resource and initialize reference tracking
        typeResources[resourceId] = resource;
        auto& refDataCounts = refCounts[std::type_index(typeid(T))];
        //Could be a race condition in the future when loading more than one instance of
        //A reference on multiple threads?
        auto refIt = refDataCounts[resourceId] = {
            .resource = resource,
            .refCount = 1
        };

        return ResourceHandle<T>(resourceId, this);
    }

    template<typename T>
    T* GetResource(const std::string& resourceId){
        //Access type-specific resource container using compile-time type information
        auto& typeResources = resources[std::type_index(typeid(T))];
        auto it = typeResources.find(resourceId);

        //If resource found, return it.
        if(it != typeResources.end()){
            return static_cast<T*>(it->second.get());
        }

        //Resource not found
        return nullptr;
    }

    template<typename T>
    bool HasResource(const std::string& resourceId){
        //Efficient existence check without resource access overhead
        auto resourceIt = resources.find(std::type_index(typeid(T)));
        return resourceIt != resources.end();
    }

    template<typename T>
    void Release(const std::string& resourceId){
        //Locate reference count entry for this resource
        auto& countMap = refCounts[std::type_index(typeid(T))];
        auto it = countMap.find(resourceId);
        if(it != countMap.end()){
            it->second.refCount--;

            //Check if the resource has no remaining references
            if(it->second.refCount <= 0){
                //Locate and unload the referenced resource across all type containers
                for(auto& [type, typeResources] : resources){
                    auto resourceIt = typeResources.find(resourceId);
                    if(resourceIt != typeResources.end()){
                        resourceIt->second->Unload();
                        typeResources.erase(resourceIt);
                        break;
                    }
                }
                //Clean up reference counting entry
                countMap.erase(it);
            }
        }
    }

    void UnloadAll(){
        //Emergency cleanup method for system shutdown or major state changes
        for(auto& [type, typeResources] : resources){
            for(auto& [id, resource] : typeResources){
                resource->Unload();
            }
            typeResources.clear();
        }
        refCounts.clear();
    }
};

template <typename T>
class ResourceHandle
{
  private:
    std::string resourceId;
    ResourceManager *resourceManager;

  public:
    ResourceHandle() : resourceManager(nullptr)
    {
    }
    ResourceHandle(const std::string &id, ResourceManager *manager) : resourceId(id), resourceManager(manager)
    {
    }

    T *Get() const
    {
        if (!resourceManager)
            return nullptr;
        return resourceManager->GetResource<T>(resourceId);
    }

    bool IsValid() const
    {
        return resourceManager && resourceManager->HasResource<T>(resourceId);
    }

    const std::string &GetID() const
    {
        return resourceId;
    }

    // Convenience Operators
    T *operator->() const
    {
        return Get();
    }

    T &operator*() const
    {
        return *Get();
    }

    operator bool() const
    {
        return IsValid();
    }
};


#endif