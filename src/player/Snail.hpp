#pragma once

#include "../precompiled.h"
#include "../core/Object.hpp"
#include "../core/Shader.hpp"
#include "../world/Terrain.hpp"
#include "../physics/ShellPhysics.hpp"

class Snail
{
public:
	Snail() = default;
	~Snail();

	// Loads the visual OBJ models and prepares the fallback placeholder mesh.
	void Init();

	// Main gameplay update. Handles input, mode transitions, terrain following,
	// shell physics, and orientation alignment to the terrain normal.
	void Update(float dt, GLFWwindow* window, const Terrain& terrain);

	// Draws the visible snail body/shell using the material shader.
	void Draw(const std::shared_ptr<Shader>& shader) const;
	// Draws the same character into the shadow depth map.
	void DrawDepth(const std::shared_ptr<Shader>& shader) const;

	// World-space logical anchor of the character. Rendering offsets are built
	// around this point, so it is also used for vegetation interactions.
	const glm::vec3& GetPosition() const { return position_; }

	bool IsShellMode() const
	{
		return mode_ == Mode::Shell;
	}

	// Normal-mode vegetation reward: temporary speed multiplier.
	void ApplySpeedBoost(float duration, float multiplier);
	// Shell-mode vegetation penalty: reduces current rolling velocity.
	void SlowShell(float multiplier);

	float GetShellSpeed() const
	{
		return ShellPhysics::GetSpeed(shell_physics_);
	}

	bool IsShellGrounded() const
	{
		return shell_physics_.grounded;
	}

private:
	// High-level character state. Retracting and Unretracting are transition
	// states driven by retract_progress_; Shell is the full rolling mode.
	enum class Mode
	{
		Normal,
		Retracting,
		Shell,
		Unretracting
	};

	void CreateBoxMesh();

	// Detects edge-triggered mode switches. The gameplay uses R as a toggle
	// between normal snail movement and shell-only physics.
	void HandleInput(float dt, GLFWwindow* window);

	// Character-style movement while the body is visible.
	void UpdateNormalMovement(float dt, GLFWwindow* window);
	// Physics-driven movement when the snail is fully inside the shell.
	void UpdateShellPhysics(float dt, GLFWwindow* window, const Terrain& terrain);
	// Advances retract_progress_ and changes Mode at the ends of the animation.
	void UpdateRetractAnimation(float dt);

	// Converts arrow-key input into a horizontal vector for shell physics.
	glm::vec3 ReadShellInput(GLFWwindow* window) const;
	// Copies tunable values from Config into a compact physics settings object.
	ShellPhysics::Settings BuildShellPhysicsSettings() const;

	void ClampToTerrainBounds();
	// Builds a terrain-aligned basis (right/up/forward) used by all model
	// matrices so the snail naturally follows slopes.
	void UpdateOrientationFromTerrain(const Terrain& terrain);

	float GetTerrainSlopeAngle(const Terrain& terrain) const;

	// Separate body/shell matrices allow the shell to sit on the back in normal
	// mode but become centered and lifted while rolling.
	glm::mat4 ComputeBodyModelMatrix() const;
	glm::mat4 ComputeShellModelMatrix() const;

	glm::mat4 BuildModelMatrix(
		const glm::vec3& localOffset,
		const glm::vec3& localScale
	) const;

	bool IsBodyVisible() const;
	bool IsShellOnly() const;

private:
	// Logical world anchor for physics and terrain height. Visual meshes are
	// offset from this point by ComputeBodyModelMatrix/ComputeShellModelMatrix.
	glm::vec3 position_{ 0.0f };
	// Local movement basis. It is rebuilt from yaw and terrain normal each frame.
	glm::vec3 forward_{ 0.0f, 0.0f, -1.0f };
	glm::vec3 right_{ 1.0f, 0.0f, 0.0f };
	glm::vec3 up_{ 0.0f, 1.0f, 0.0f };

	// Terrain-aligned transform basis for the character.
	glm::mat4 orientation_{ 1.0f };

	// Heading around the Y axis. Shell physics updates it from velocity.
	float yaw_ = 0.0f;

	float move_speed_ = Config::snail_move_speed;
	float turn_speed_ = Config::snail_turn_speed;

	// Small creep animation values for normal movement.
	float animation_time_ = 0.0f;
	float creep_amount_ = 0.0f;
	bool is_moving_ = false;

	Mode mode_ = Mode::Normal;

	// 0 = body fully out, 1 = fully inside shell.
	float retract_progress_ = 0.0f;
	float retract_speed_ = Config::snail_retract_speed;
	bool r_was_pressed_ = false;

	// Complete rolling-shell simulation state.
	ShellPhysics::State shell_physics_;

	float shell_radius_ = Config::snail_shell_radius;
	float shell_turn_strength_ = Config::shell_turn_strength;
	float max_shell_climb_slope_ = Config::shell_max_climb_slope;

	// Temporary boost applied after eating vegetation.
	float speed_boost_timer_ = 0.0f;
	float speed_boost_multiplier_ = 1.0f;

	// Placeholder cube mesh used only if OBJ loading fails.
	GLuint vao_ = 0;
	GLuint vbo_ = 0;
	GLuint ebo_ = 0;

	std::vector<float> vertices_;
	std::vector<unsigned int> indices_;

	// Real production meshes loaded from assets/models.
	std::unique_ptr<Object> slug_object_ = nullptr;
	std::unique_ptr<Object> shell_object_ = nullptr;
};
