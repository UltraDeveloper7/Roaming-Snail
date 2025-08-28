#include "../precompiled.h"
#include "Light.hpp"
#include "../core/Loader.hpp"


Light::Light(const std::string& path, const glm::vec3& position, const glm::vec3& color)
    : Object(path), position_(position), color_(color), is_on_(false), scale_(glm::vec3(1.0f))
{
}

void Light::SetPosition(const glm::vec3& position)
{
    position_ = position;
	Translate(position);
}

void Light::SetColor(const glm::vec3& color)
{
    color_ = color;
}

void Light::Toggle()
{
    is_on_ = !is_on_;
    color_ = is_on_ ? glm::vec3(15.0f, 12.0f, 8.0f) : glm::vec3(0.0f, 0.0f, 0.0f);
}

const glm::vec3& Light::GetPosition() const
{
    return position_;
}

const glm::vec3& Light::GetColor() const
{
    return color_;
}

bool Light::IsOn() const
{
    return is_on_;
}

void Light::SetScale(const glm::vec3& scale) {
    scale_ = scale;
	Scale(scale);
}

glm::vec3 Light::GetScale() const {
    return scale_;
}