#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Component.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

class Component;

class Entity
{
  public:
    template <typename T, typename... Args> T *AddComponent(Args &&...args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        size_t typeID = Component::GetTypeID<T>();

        // Check if component of this type already exists in map
        // This limits us to one of each component per entity? What if we want multiple
        // Limitations breed creativty?
        auto it = componentMap.find(typeID);
        if (it != componentMap.end())
        {
            return static_cast<T *>(it->second);
        }
        // Otherwise add component
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T *componentPtr = component.get();
        componentMap[typeID] = componentPtr;
        components.push_back(std::move(component));
        return componentPtr;
    }

    template <typename T> T *GetComponent()
    {
        size_t typeID = Component::GetTypeID<T>();
        auto it = componentMap.find(typeID);
        if (it != componentMap.end())
        {
            return static_cast<T *>(it->second);
        }
        return nullptr;
    }

    template <typename T> bool RemoveComponent()
    {
        size_t typeID = Component::GetTypeID<T>();
        auto it = componentMap.find(typeID);
        if (it != componentMap.end())
        {
            Component *componentPtr = it->second;
            componentMap.erase(it);
            for (auto compIt = components.begin(); compIt != components.end(); ++compIt)
            {
                if (compIt->get() == componentPtr)
                {
                    components.erase(compIt);
                    return true;
                }
            }
        }
        return false;
    }

    void Update(float deltaTime);

  private:
    std::vector<std::unique_ptr<Component>> components;
    std::unordered_map<size_t, Component *> componentMap;
};

#endif