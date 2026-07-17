#ifndef __MATERIAL_H__
#define __MATERIAL_H_

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <string>

class Material {
public:
  void Bind() {}
  void SetUniform(std::string attr, glm::mat4 buffer) {}
};

#endif