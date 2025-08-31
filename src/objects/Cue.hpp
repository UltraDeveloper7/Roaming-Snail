#pragma once
#include "../precompiled.h"
#include "Ball.hpp"
#include "CueBallMap.hpp"

class Cue final : public Object
{
public:
	Cue(std::shared_ptr<CueBallMap> cue_ball_map);
	void HandleShot(const std::shared_ptr<Ball>& white_ball, float dt);
	void PlaceAtBall(const std::shared_ptr<Ball>& ball);

	// Add the GetCueBallMap method
	std::shared_ptr<CueBallMap> GetCueBallMap() const { return cue_ball_map_; }

	glm::vec3 AimDir() const {
		const glm::vec3 up(0, 1, 0);
		const glm::vec3 cueDir(std::sin(angle_), 0.0f, std::cos(angle_));
		return -glm::normalize(glm::cross(cueDir, up)); // matches HandleShot()
	}

private:
	bool power_changed_{false};
	std::shared_ptr<CueBallMap> cue_ball_map_;
};
