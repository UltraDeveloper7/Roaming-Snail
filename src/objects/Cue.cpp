#include "../precompiled.h"
#include "Cue.hpp"

namespace {
	// tiny helpers to keep intent clear
	inline glm::vec3 dirFromAngle(float a) { return { std::sin(a), 0.0f, std::cos(a) }; }
	inline glm::vec3 rotAxisFromAngle(float a) { return { std::cos(a) * 0.1f, 1.0f, -std::sin(a) * 0.1f }; }
}

Cue::Cue(std::shared_ptr<CueBallMap> cue_ball_map)
	: Object(Config::cue_path), cue_ball_map_(cue_ball_map)
{
}

void Cue::HandleShot(const std::shared_ptr<Ball>& white_ball, const float dt)
{
    GLFWwindow* window = glfwGetCurrentContext();

    // Precompute vectors used in multiple branches
    const glm::vec3 cue_dir = dirFromAngle(angle_);               // forward along cue
    const glm::vec3 cue_rot_axis = rotAxisFromAngle(angle_);           // tilt axis for small arc
    const glm::vec3 cue_displace = glm::cross(cue_dir, cue_rot_axis);  // local sideways for pull/push
    const glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    const glm::vec3 power_vec = glm::cross(cue_dir, up);            // points from cue tip into cue ball

    // Distance from cue to white ball drives power
    float power = glm::distance(translation_, white_ball->translation_);

    // Read keys once
    const bool kLeft = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool kRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool kUp = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool kDown = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    const bool kSpace = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    // Rotate cue around the ball (left/right) — only when we haven't changed power this frame
    if (!power_changed_) {
        const float dtheta = Config::cue_rot_speed * dt; // radians this frame
        if (kLeft) {
            Rotate(cue_rot_axis, dtheta);
            Translate(Ball::radius_ * dtheta * cue_dir);   // arc length = R * dtheta
        }
        if (kRight) {
            Rotate(cue_rot_axis, -dtheta);
            Translate(-Ball::radius_ * dtheta * cue_dir);
        }
    }

    // Adjust "pulled back" amount which affects shot power (up/down)
    // Keep the exact thresholds from original code.
    if (kUp) {
        if (power > Ball::radius_) {
            Translate(-cue_displace * dt);
            power_changed_ = true;
        }
    }

    if (kDown) {
        if (power <= 0.5f) {
            Translate(cue_displace * dt);
            power_changed_ = true;
        }
    }

    // Fire!
    if (kSpace) {
        const float shot_power = power * Config::power_coeff;
        const glm::vec2 spin = cue_ball_map_ ? cue_ball_map_->GetSpin() : glm::vec2(0.0f);

        // Direction is the same as before (negative power_vec)
        white_ball->Shot(-power_vec * shot_power, spin);

        // Reset so next left/right rotation is allowed without needing a new power adjust
        power_changed_ = false;
    }
}

void Cue::PlaceAtBall(const std::shared_ptr<Ball>& ball)
{
	translation_.x = ball->translation_.x + Ball::radius_ + Config::min_change;
	translation_.y = Ball::radius_;
	translation_.z = ball->translation_.z;

	angle_ = glm::pi<float>();
	rotation_axis_ = glm::vec3(-0.1f, 1.0f, 0.0f);
}

