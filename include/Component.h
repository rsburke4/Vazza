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
  enum class State{
    Uninitialized,
    Initializing,
    Active,
    Destroying,
    Destroyed
  };

  virtual ~Component();
  void Initialize();
  void Destroy();
  bool IsActive() const{ return state == State::Active; }

  void SetOwner(Entity *entity) { owner = entity; }
  Entity *GetOwner() const { return owner; }

protected:
  State state = State::Uninitialized;
  Entity *owner = nullptr;
  virtual void OnDestroy(){}
  virtual void OnInitialize(){}
  virtual void Update(float deltaTime) {}
  virtual void Render() {}

  friend class Entity; //Allows friend class to call protected methods
};

// TODO: Add constructor and set default values for private varaibles
class TransformComponent : public Component {
public:
  void SetPosition(const glm::vec3 &pos);
  void SetRotation(const glm::quat &rot);
  void SetScale(const glm::vec3 &s);

  const glm::vec3 &GetPosition() const;
  const glm::quat &GetRotation() const;
  const glm::vec3 &GetScale() const;
  glm::mat4 GetTransformMatrix() const;

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
  void SetMesh(Mesh *m);
  void SetMaterial(Material *mat);

  Mesh *GetMesh() const;
  Material *GetMaterial() const;

  void Render() override;
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
  void SetPerspective(float fov, float aspect, float near, float far);
  glm::mat4 GetViewMatrix() const;
  glm::mat4 GetProjectionMatrix() const;
};

#endif