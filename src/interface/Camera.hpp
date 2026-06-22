#pragma once

#include "../precompiled.h"

class Camera
{
public:
	Camera() = default;

	// Sets the start position/orientation from Config.
	void Init();
	// Handles camera movement/rotation and rebuilds the view matrix.
	void Update(float frame_time);
	// Rebuilds projection after window resize.
	void UpdateProjectionMatrix(int width, int height);

	const glm::mat4& GetViewMatrix() const { return view_matrix_; }
	const glm::mat4& GetProjectionMatrix() const { return projection_matrix_; }
	const glm::vec3& GetPosition() const { return position_; }

	void SetPosition(const glm::vec3& position) { position_ = position; }

private:
	// Converts yaw/pitch angles into a forward direction vector.
	glm::vec3 DirectionFromAngles() const;
	// Useful when a desired look direction should become camera angles.
	void SetAnglesFromDirection(const glm::vec3& direction);
	// WASD-style movement along a direction vector.
	void Move(GLFWwindow* window, const glm::vec3& direction, float factor);
	// Mouse-look rotation with optional cursor lock.
	void Rotate(GLFWwindow* window, float sensitivity);

private:
	glm::mat4 projection_matrix_{ 1.0f };
	glm::mat4 view_matrix_{ 1.0f };

	// Previous mouse position for relative rotation.
	glm::vec2 prior_cursor_{ 0.0f };
	glm::vec3 position_{ Config::camera_start_position };

	// Euler angles used to compute the camera forward direction.
	float pitch_{ Config::camera_start_pitch };
	float yaw_{ Config::camera_start_yaw };

	// Cursor flags prevent first-frame mouse jumps and allow locking toggle.
	bool cursor_initialized_ = false;
	bool cursor_locked_ = true;
	bool l_was_pressed_ = false;
};
