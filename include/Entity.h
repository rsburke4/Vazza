#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <iostream>
#include <vector>
#include <memory>
#include "Component.h"

class Entity{
    public:
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args){
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T* componentPtr = component.get();
            components.push_back(std::move(component));
            return componentPrt;
        }

        template<typename T>
        T* GetComponent(){
            for(auto& component : components){
                if(T* result = dynamic_cast<T*>(component.get())){
                    return result;
                }
            }
            return nullptr;
        }

        void Update(float deltaTime){
            for(auto& component : components){
                component->Update(deltaTime);
            }
        }

    private:
        std::vector<std::unique_ptr<Component>> components;
};

#endif