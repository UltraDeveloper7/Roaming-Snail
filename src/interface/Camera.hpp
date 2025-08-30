#pragma once
#include "../precompiled.h"
#include "../core/Shader.hpp"
#include "../core/Object.hpp"
#include "Logger.hpp"

// Forward declaration
class World;

class Camera
{
public:
	Camera() = default;

	void Init();
	void UpdateViewMatrix(float frame_time);
	void UpdateProjectionMatrix(int width, int height);
	void UpdateMain(const std::shared_ptr<Shader>& main_shader, const World& world) const;
	void UpdateBackground(const std::shared_ptr<Shader>& background_shader) const;
	void SetTopDownView(bool enabled);
	bool IsTopDownView() const;

	// Added getter methods
	glm::mat4 GetViewMatrix() const { return view_matrix_; }
	glm::mat4 GetProjectionMatrix() const { return projection_matrix_; }
	glm::vec3 GetCursorWorldPosition() const;

	// Return (origin, direction) of a picking ray
	std::pair<glm::vec3, glm::vec3> GetMouseRay(float mouseX, float mouseY, int screenW, int screenH) const;


private:

	void Move(GLFWwindow* window, const glm::vec3& direction, float factor);
	void Rotate(GLFWwindow* window, float factor);
	bool top_down_view_{ false };

	glm::mat4 projection_matrix_{};
	glm::mat4 view_matrix_{};

	glm::vec2 prior_cursor_{0.0f};
	glm::vec3 position_{0.0f, 1.3f, 0.0f};
	float pitch_{-glm::half_pi<float>()};
	float yaw_{0.0f};

	// internal: avoid first-frame mouse jump after enabling rotation
	bool cursor_initialized_ = false;
};
