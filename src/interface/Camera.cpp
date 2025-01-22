#include "../stdafx.h"
#include "Camera.hpp"
#include "../objects/World.hpp"

void Camera::Init()
{
	UpdateProjectionMatrix(Config::width, Config::height);
	view_matrix_ = glm::mat4(1.0f);
}

void Camera::UpdateViewMatrix(const float frame_time)
{
	if (!top_down_view_)
	{
		GLFWwindow* window = glfwGetCurrentContext();

		constexpr glm::vec3 up = { 0.0f, 1.0f, 0.0f };
		const glm::vec3 direction = glm::normalize(glm::vec3(
			cos(yaw_) * cos(pitch_),
			sin(pitch_),
			sin(yaw_) * cos(pitch_)));

		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		{
			Move(window, direction, Config::movement_speed * frame_time);
			Rotate(window, Config::rotation_speed * frame_time);
		}

		view_matrix_ = glm::lookAt(position_, position_ + direction, up);
	}
	else
	{
		// For top-down view, look straight down
		view_matrix_ = glm::lookAt(position_, position_ + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}
}

void Camera::UpdateProjectionMatrix(const int width, const int height)
{
	const float aspect_ratio = width > 0 && height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
	projection_matrix_ = glm::perspective(Config::fov, aspect_ratio, Config::near_clip, Config::far_clip);
}

void Camera::UpdateMain(const std::shared_ptr<Shader>& main_shader, const World& world) const {
	main_shader->Bind();
	main_shader->SetMat4(view_matrix_, "viewMatrix");
	main_shader->SetMat4(projection_matrix_, "projectionMatrix");
	main_shader->SetVec3(position_, "cameraPos");

	const int max_shader_lights = Config::max_shader_lights; // Matches `lights[14]` in the shader

	// 1. Set non-physical lights
	for (int i = 0; i < Config::light_count; i++) {
		if (i >= max_shader_lights) break;  // Prevent overflow
		const float light_position_x = i % 2 ? 2.0f * i : -2.0f * i;
		main_shader->SetVec3(glm::vec3(20.0f), std::string("lights[") + std::to_string(i) + "].color");
		main_shader->SetVec3(glm::vec3(light_position_x, 2.0f, 0.0f), std::string("lights[") + std::to_string(i) + "].position");
		main_shader->SetBool(true, "lights[" + std::to_string(i) + "].isOn");
	}

	// 2. Set physical lights
	const auto& lights = world.GetLights();
	for (int i = 0; i < lights.size(); i++) {
		int shader_index = Config::light_count + i;
		if (shader_index >= max_shader_lights) break;  // Prevent overflow
		main_shader->SetVec3(lights[i]->GetColor(), std::string("lights[") + std::to_string(shader_index) + "].color");
		main_shader->SetVec3(lights[i]->GetPosition(), std::string("lights[") + std::to_string(shader_index) + "].position");
		main_shader->SetBool(lights[i]->IsOn(), "lights[" + std::to_string(shader_index) + "].isOn");
	}

	// 3. Set `lightCount` to the correct number
	int total_lights = Config::light_count + lights.size();
	if (total_lights > max_shader_lights) {
		Logger::Log("Warning: Total light count exceeds shader capacity! Truncating to " + std::to_string(max_shader_lights));
		total_lights = max_shader_lights;
	}

	main_shader->SetInt(total_lights, "lightCount");

	if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		// example: treat the camera as a dynamic light
		if (Config::light_count < max_shader_lights)
		{
			main_shader->SetVec3(glm::vec3(10.0f), std::string("lights[") + std::to_string(Config::light_count) + "].color");
			main_shader->SetVec3(position_, std::string("lights[") + std::to_string(Config::light_count) + "].position");
			main_shader->SetBool(true, "lights[" + std::to_string(Config::light_count) + "].isOn");
			main_shader->SetInt(Config::light_count + 1, "lightCount");
		}
	}

	main_shader->Unbind();
}


void Camera::UpdateBackground(const std::shared_ptr<Shader>& background_shader) const
{
	background_shader->Bind();
	background_shader->SetMat4(view_matrix_, "viewMatrix");
	background_shader->SetMat4(projection_matrix_, "projectionMatrix");
	background_shader->Unbind();
}

void Camera::Move(GLFWwindow* window, const glm::vec3& direction, const float factor)
{
	constexpr glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	const glm::vec3 right = glm::normalize(glm::cross(direction, up));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		position_ += direction * factor;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		position_ -= direction * factor;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		position_ -= right * factor;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		position_ += right * factor;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		position_ += up * factor;
	}
	else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		position_ -= up * factor;
	}

	if (Config::bound_camera)
		position_ = glm::clamp(position_, Config::camera_min_position, Config::camera_max_position);
}

void Camera::Rotate(GLFWwindow* window, const float factor)
{
	double current_cursor_x, current_cursor_y;
	glfwGetCursorPos(window, &current_cursor_x, &current_cursor_y);

	const glm::vec2 delta = { static_cast<float>(current_cursor_x) - prior_cursor_.x, static_cast<float>(current_cursor_y) - prior_cursor_.y };
	prior_cursor_ = { current_cursor_x, current_cursor_y };

	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		pitch_ -= delta.y * factor;
		yaw_ += delta.x * factor;

		pitch_ = glm::clamp(pitch_, -1.5f, 1.5f);
		yaw_ = glm::mod(yaw_, glm::two_pi<float>());
	}
}

void Camera::SetTopDownView(bool enabled) {
	top_down_view_ = enabled;
	if (top_down_view_) {
		// Position the camera above the table looking downward
		position_ = glm::vec3(0.0f, 1.6f, 0.0f);
		pitch_ = -glm::half_pi<float>();         // Look straight down
		yaw_ = 0.0f;
	}
	else {
		// Reset to default position and orientation
		position_ = glm::vec3(0.0f, 1.3f, 0.0f);
		pitch_ = -glm::half_pi<float>();
	}
}

bool Camera::IsTopDownView() const {
	return top_down_view_;
}

glm::vec3 Camera::GetCursorWorldPosition() const
{
	double cursor_x, cursor_y;
	glfwGetCursorPos(glfwGetCurrentContext(), &cursor_x, &cursor_y);

	glm::vec4 viewport = glm::vec4(0.0f, 0.0f, Config::width, Config::height);
	glm::vec3 screen_pos = glm::vec3(cursor_x, Config::height - cursor_y, 0.0f);
	glm::vec3 world_pos = glm::unProject(screen_pos, view_matrix_, projection_matrix_, viewport);

	return world_pos;
}

/**
 * Build a ray from screen coords to world space:
 * 1) Convert to NDC
 * 2) Inverse projection => view space
 * 3) Inverse view => world space
 */
std::pair<glm::vec3, glm::vec3> Camera::GetMouseRay(float mouseX, float mouseY, int screenW, int screenH) const
{
	// Convert [0..screenW, 0..screenH] to Normalized Device Coordinates:
	float ndcX = (2.f * mouseX) / float(screenW) - 1.f;
	float ndcY = 1.f - (2.f * mouseY) / float(screenH);

	glm::vec4 rayClip(ndcX, ndcY, -1.f, 1.f);
	glm::mat4 invProj = glm::inverse(projection_matrix_);
	glm::vec4 rayEye = invProj * rayClip;
	rayEye.z = -1.f; // forward
	rayEye.w = 0.f;

	glm::mat4 invView = glm::inverse(view_matrix_);
	glm::vec4 rayWorld = invView * rayEye;

	glm::vec3 dir = glm::normalize(glm::vec3(rayWorld));
	glm::vec3 origin = position_; // camera position
	return { origin, dir };
}