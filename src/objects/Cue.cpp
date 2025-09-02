#include "../precompiled.h"
#include "Cue.hpp"

namespace {
    // Helpers
    inline glm::vec3 dirFromAngle(float a) { return { std::sin(a), 0.0f, std::cos(a) }; }
    inline glm::vec3 rotAxisFromAngle(float a) { return { std::cos(a) * 0.1f, 1.0f, -std::sin(a) * 0.1f }; }

    // Elevation params
    constexpr float kMaxElevDeg = 25.0f;                 // hard max
    constexpr float kMaxElevRad = glm::radians(kMaxElevDeg);
    constexpr float kElevStepDeg = 2.5f;                  // per key press
    constexpr float kElevStepRad = glm::radians(kElevStepDeg);
    constexpr float kElevSpeed = glm::radians(180.0f);  // how fast to animate toward target (deg/s)
}

Cue::Cue(std::shared_ptr<CueBallMap> cue_ball_map)
    : Object(Config::cue_path), cue_ball_map_(cue_ball_map)
{
}

void Cue::HandleShot(const std::shared_ptr<Ball>& white_ball, const float dt)
{
    GLFWwindow* window = glfwGetCurrentContext();

    // ---------- precompute common vectors (your style kept) ----------
    const glm::vec3 cue_dir = dirFromAngle(angle_);           // forward along cue (yaw)
    const glm::vec3 cue_rot_axis = rotAxisFromAngle(angle_);       // arc axis around the ball
    const glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    const glm::vec3 power_vec = glm::cross(cue_dir, up);        // tip -> ball (flat)
    const glm::vec3 cue_displace = glm::cross(cue_dir, cue_rot_axis); // pull/push offset
    const glm::vec3 right_axis = glm::normalize(glm::cross(power_vec, up)); // local right

    // Elevated strike dir: tilt power_vec around local right (butt up, tip on ball)
    auto ElevatedStrikeDir = [&](float elev_rad) -> glm::vec3 {
        const glm::quat q = glm::angleAxis(-elev_rad, right_axis);
        return glm::normalize(q * power_vec);
        };

    // Power = tip distance to white
    float power = glm::distance(translation_, white_ball->translation_);

    // Keys (one-shot step on edge)
    const bool left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    const bool right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    const bool upKey = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    const bool downKey = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
    const bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    // Press-to-step elevation: R raises, F lowers (no need to hold)
    const bool r_now = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    const bool f_now = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;

    if (r_now && !r_was_down_) {
        target_elevation_angle_ = glm::min(target_elevation_angle_ + kElevStepRad, kMaxElevRad);
    }
    if (f_now && !f_was_down_) {
        target_elevation_angle_ = glm::max(target_elevation_angle_ - kElevStepRad, 0.0f); // 0° = parallel (lowest stop)
    }
    r_was_down_ = r_now;
    f_was_down_ = f_now;

    // Smoothly animate current angle toward target
    const float diff = target_elevation_angle_ - elevation_angle_;
    const float maxDelta = kElevSpeed * dt;
    if (std::abs(diff) <= maxDelta) elevation_angle_ = target_elevation_angle_;
    else elevation_angle_ += (diff > 0.0f ? maxDelta : -maxDelta);

    // Rotate cue around ball (left/right) — unchanged
    if (!power_changed_) {
        const float dtheta = Config::cue_rot_speed * dt;
        if (left) {
            Rotate(cue_rot_axis, dtheta);
            Translate(Ball::radius_ * dtheta * cue_dir);
        }
        if (right) {
            Rotate(cue_rot_axis, -dtheta);
            Translate(-Ball::radius_ * dtheta * cue_dir);
        }
    }

    // Pull / Push — unchanged
    if (upKey && power > Ball::radius_) {
        Translate(-cue_displace * dt);
        power_changed_ = true;
    }
    if (downKey && power <= 0.5f) {
        Translate(cue_displace * dt);
        power_changed_ = true;
    }

    // Keep tip on ball height (prevents cloth clipping)
    translation_.y = Ball::radius_;

    // Fire
    if (space) {
        const float shot_power = power * Config::power_coeff;
        const glm::vec2 spin = cue_ball_map_ ? cue_ball_map_->GetSpin() : glm::vec2(0.0f);
        const glm::vec3 power_vec_elev = ElevatedStrikeDir(elevation_angle_);
        white_ball->Shot(-power_vec_elev * shot_power, spin);
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

    // Reset elevation when re-placing
    elevation_angle_ = 0.0f;
    target_elevation_angle_ = 0.0f;
    r_was_down_ = f_was_down_ = false;
}

// Visual tilt (pivot at tip)
void Cue::Draw(const std::shared_ptr<Shader>& shader)
{
    glm::mat4 model = GetModelMatrix();

    // Rebuild local right axis from yaw
    const glm::vec3 cue_dir = dirFromAngle(angle_);
    const glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    const glm::vec3 power_vec = glm::cross(cue_dir, up);
    const glm::vec3 right_axis = glm::normalize(glm::cross(power_vec, up));

    // Rotate about tip by -elevation (butt up)
    model = glm::rotate(model, -elevation_angle_, right_axis);

    shader->Bind();
    shader->SetMat4(model, "modelMatrix");

    for (const auto& mesh : meshes_) {
        const auto material = materials_[mesh->GetMaterialId()];
        material->Bind(shader);
        mesh->Bind();
        mesh->Draw();
        mesh->Unbind();
        material->Unbind(shader);
    }
    shader->Unbind();
}
