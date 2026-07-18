#include "Entity.h"

void Entity::Update(float deltaTime) {
    for (auto &component : components) {
      component->Update(deltaTime);
    }
  }