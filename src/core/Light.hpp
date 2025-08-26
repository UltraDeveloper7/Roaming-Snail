#pragma once
#include "../stdafx.h"
#include "Object.hpp"
#include "../core/Material.hpp" 

class Light : public Object {
public:
    Light(const std::string& path, const glm::vec3& position, const glm::vec3& color);

	// Setters
    void SetPosition(const glm::vec3& position);
    void SetColor(const glm::vec3& color);
    void SetScale(const glm::vec3& scale); 

    // Getters
    const glm::vec3& GetPosition() const;
    const glm::vec3& GetColor() const;
    glm::vec3 GetScale() const;

    void Toggle();

    bool IsOn() const;

private:
    glm::vec3 position_;
    glm::vec3 color_;
    glm::vec3 scale_ = glm::vec3(1.0f); // Default scale (no scaling)
    bool is_on_;
};