#pragma once
#include "../precompiled.h"
#include "Object.hpp"
#include "../objects/Table.hpp"
#include "../objects/Cue.hpp"
#include "../objects/CueBallMap.hpp"
#include "../objects/Ball.hpp"
#include "../objects/Ceiling.hpp"
#include "Light.hpp"
#include "../gameplay/GameState.hpp"
#include "../gameplay/GameRules.hpp"

// Forward declarations
class CueBallMap;
class Camera;

class World
{
public:
	World(std::shared_ptr<CueBallMap> cue_ball_map, Camera& camera);

	void Update(float dt, bool in_game);
	void Draw(const std::shared_ptr<Shader>& shader) const;

	// Initialization & Reset
	void Init() const;
	void Reset() const;

	//Collision Handling
	void HandleBallsCollision(int number) ;
	void HandleHolesFall(int number) const;
	void HandleBoundsCollision(const int number) const;

	[[nodiscard]] bool AreBallsInMotion() const;

	// Add method to toggle lights
	void ToggleLight(int index); 

	// Add getter method for lights
	const std::vector<std::shared_ptr<Light>>& GetLights() const { return lights_; }

	// HUD accessors from gameplay state
	const std::vector<Player>& GetPlayers() const { return state_.Players(); }
	int GetCurrentPlayerIndex() const { return state_.CurrentPlayerIndex(); }


	void ResetPlayerIndex() { state_.ResetPlayerIndex(); }
	void ResetGame();


	float GetShotClock() const { return state_.ShotClock(); }
	bool IsGameOver() const { return state_.IsGameOver(); }
	std::string GetMessage() const { return state_.Message(); }


	// expose balls for rules module
	const std::vector<std::shared_ptr<Ball>>& GetBalls() const { return balls_; }


private:
	void InitializeLights();


	// Input helper
	void PlaceCueBallWithMouse();
	glm::vec3 ClampCueBallPosition(const glm::vec3& desiredPos) const;


	Camera& camera_;


	std::shared_ptr<Table> table_ = nullptr;
	std::shared_ptr<Cue> cue_ = nullptr;
	std::vector<std::shared_ptr<Ball>> balls_{};
	std::vector<std::shared_ptr<Light>> lights_{};
	std::shared_ptr<Ceiling> ceiling_ = nullptr;


	GameState state_{};
	GameRules rules_{};
};