#pragma once
#include "../precompiled.h"
#include "Ball.hpp"

class Table final : public Object
{
public:
	Table();

	[[nodiscard]] const std::vector<glm::vec3>& GetHoles() const { return holes_; }
	[[nodiscard]] float GetBoundX() const { return bound_x_; }
	[[nodiscard]] float GetBoundZ() const { return bound_z_; }

	inline static constexpr float hole_radius_{ 0.07f };
	inline static constexpr float hole_bottom_{ -0.14324f };
	inline static constexpr float bound_x_{1.35f - Ball::radius_ - 0.042f};
	inline static constexpr float bound_z_{0.7f - Ball::radius_ - 0.042f};

private:
	std::vector<glm::vec3> holes_{};
};
