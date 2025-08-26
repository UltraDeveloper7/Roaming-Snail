#pragma once
#include "../stdafx.h"
#include "../core/Object.hpp"

class Ceiling : public Object {
public:
    explicit Ceiling(const std::string& path, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation);
    void SetPosition(const glm::vec3& position);
    void SetScale(const glm::vec3& scale);
    void SetRotation(const glm::vec3& rotation);
    const glm::vec3& GetPosition() const;
    const glm::vec3& GetScale() const;
    const glm::vec3& GetRotation() const;

private:
    glm::vec3 position_;
    glm::vec3 scale_;
    glm::vec3 rotation_;
};
