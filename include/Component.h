#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include <memory>
#include <string>

class Entity;

class Component {
public:
  virtual void Initialize() {}
  virtual void Update(float deltaTime) {}
  virtual void Render() {}

  void SetOwner(Entity *entity) { owner = entity; }
  Entity *GetOwner() const { return owner; }

protected:
  Entity *owner = nullptr;
};

// TODO: Add constructor and set default values for private varaibles
class TransformComponent : public Component {
public:
  void SetPosition(const glm::vec3 &pos) {
    position = pos;
    transformDirty = true;
  }
  void SetRotation(const glm::quat &rot) {
    rotation = rot;
    transformDirty = true;
  }
  void SetScale(const glm::vec3 &s) {
    scale = s;
    transformDirty = true;
  }

  const glm::vec3 &GetPosition() const { return position; }
  const glm::quat &GetRotation() const { return rotation; }
  const glm::vec3 &GetScale() const { return scale; }
  glm::mat4 GetTransformMatrix() const {
    if (transformDirty) {
      glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
      glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

      transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
      transformDirty = false;
    }
    return transformMatrix;
  }

private:
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f);

  mutable glm::mat4 transformMatrix = glm::mat4(1.0f);
  mutable bool transformDirty = true;
};

class MeshComponent : public Component {
private:
  Mesh *mesh = nullptr;
  Material *material = nullptr;

public:
  MeshComponent(Mesh *m, Material *mat) : mesh(m), material(mat) {}

  void SetMesh(Mesh *m) { mesh = m; }
  void SetMaterial(Material *mat) { material = mat; }

  Mesh *GetMesh() const { return mesh; }
  Material *GetMaterial() const { return material; }

  void Render() override {
    // TODO: Add default "missing" material
    if (!mesh || !material)
      return;

    // Get transform component
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
      return;

    // Render mesh with material and transform
    material->Bind();
    material->SetUniform("modelMatrix", transform->GetTransformMatrix());
    mesh->Render();
  }
};

// TODO: Should this be an entity? Are entities simply containers for
// components?
// TODO: Orthogonal projection
class CameraComponent : public Component {
private:
  float fieldOfView = 45.0f;
  float aspectRatio = 16.0f / 9.0f;
  float nearPlane = 0.1f;
  float farPlane = 1000.0f;

  glm::mat4 viewMatrix = glm::mat4(1.0f);
  mutable glm::mat4 projectionMatrix = glm::mat4(1.0f);
  mutable bool projectionDirty;

public:
  void SetPerspective(float fov, float aspect, float near, float far) {
    fieldOfView = fov;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;
    projectionDirty = true;
  }

  glm::mat4 GetViewMatrix() const {
    TransformComponent *transform =
        GetOwner()->GetComponent<TransformComponent>();
    if (transform) {
      glm::vec3 position = transform->GetPosition();
      glm::quat rotation = transform->GetRotation();

      // Cam forward (local -Z)
      glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
      // Cam up (local +Y)
      glm::vec3 up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);

      return glm::lookAt(position, position + forward, up);
    }
    return glm::mat4(1.0f);
  }

  glm::mat4 GetProjectionMatrix() const {
    if (projectionDirty) {
      projectionMatrix = glm::perspective(glm::radians(fieldOfView),
                                          aspectRatio, nearPlane, farPlane);
      projectionDirty = false;
    }
  }
};

#endif