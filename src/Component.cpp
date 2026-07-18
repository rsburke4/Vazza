#include "Component.h"
#include "Entity.h"

size_t ComponentTypeIDSystem::nextTypeID = 0;

Component::~Component() {
  if (state != State::Destroyed) {
    OnDestroy();
    state = State::Destroyed;
  }
}

void Component::Initialize() {
  if (state != State::Uninitialized) {
    state = State::Initializing;
    OnInitialize();
    state = State::Active;
  }
}

void Component::Destroy() {
  if (state == State::Active) {
    state = State::Destroying;
    OnDestroy();
    state = State::Destroyed;
  }
}

void TransformComponent::SetPosition(const glm::vec3 &pos) {
  position = pos;
  transformDirty = true;
}
void TransformComponent::SetRotation(const glm::quat &rot) {
  rotation = rot;
  transformDirty = true;
}
void TransformComponent::SetScale(const glm::vec3 &s) {
  scale = s;
  transformDirty = true;
}

const glm::vec3 &TransformComponent::GetPosition() const { return position; }
const glm::quat &TransformComponent::GetRotation() const { return rotation; }
const glm::vec3 &TransformComponent::GetScale() const { return scale; }

glm::mat4 TransformComponent::GetTransformMatrix() const {
  if (transformDirty) {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    transformDirty = false;
  }
  return transformMatrix;
}

void MeshComponent::SetMesh(Mesh *m) { mesh = m; }
void MeshComponent::SetMaterial(Material *mat) { material = mat; }

Mesh *MeshComponent::GetMesh() const { return mesh; }
Material *MeshComponent::GetMaterial() const { return material; }

void MeshComponent::Render() {
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

void CameraComponent::SetPerspective(float fov, float aspect, float near,
                                     float far) {
  fieldOfView = fov;
  aspectRatio = aspect;
  nearPlane = near;
  farPlane = far;
  projectionDirty = true;
}

glm::mat4 CameraComponent::GetViewMatrix() const {
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

glm::mat4 CameraComponent::GetProjectionMatrix() const {
  if (projectionDirty) {
    projectionMatrix = glm::perspective(glm::radians(fieldOfView), aspectRatio,
                                        nearPlane, farPlane);
    projectionDirty = false;
  }
  return projectionMatrix;
}