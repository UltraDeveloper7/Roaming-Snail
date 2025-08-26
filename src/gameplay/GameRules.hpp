#pragma once
#include "../stdafx.h"



class Ball;
class GameState;


class GameRules {
public:
	// Call once when balls settle (not moving). Handles fouls, scoring, win/lose, turn switch.
	void EvaluateEndOfShot(const std::vector<std::shared_ptr<Ball>>& balls, GameState& state);


private:
	static bool AreAllGroupBallsPocketed(const std::vector<std::shared_ptr<Ball>>& balls, int groupType);
};