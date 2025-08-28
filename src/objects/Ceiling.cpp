#include "../precompiled.h"
#include "Ceiling.hpp"
#include "../core/Loader.hpp"

Ceiling::Ceiling(const std::string& path, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation)
    : Object(path), position_(position), scale_(scale), rotation_(rotation)
{
    SetPosition(position);
    SetScale(scale);
    SetRotation(rotation);
}

void Ceiling::SetPosition(const glm::vec3& position)
{
    position_ = position;
    Translate(position);
}

void Ceiling::SetScale(const glm::vec3& scale)
{
    scale_ = scale;
    Scale(scale);
}

void Ceiling::SetRotation(const glm::vec3& rotation)
{
    rotation_ = rotation;
    Rotate(rotation, glm::radians(90.0f)); // Rotate 90 degrees
}

const glm::vec3& Ceiling::GetPosition() const
{
    return position_;
}

const glm::vec3& Ceiling::GetScale() const
{
    return scale_;
}

const glm::vec3& Ceiling::GetRotation() const
{
    return rotation_;
}
