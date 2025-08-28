#pragma once
#include "../precompiled.h"
#include "../core/Object.hpp"
#include "../interface/Camera.hpp"

class CueBallMap : public Object {
public:
    CueBallMap(Camera& camera, GLFWwindow* window);
    void Draw();
    void UpdateWindowSize();
    void HandleMouseInput(GLFWwindow* window);
    glm::vec2 GetSpin() const;
    void ResetAim();
    void SetVisible(bool visible);
    [[nodiscard]] bool IsVisible() const { return is_visible_; }
    [[nodiscard]] bool IsDragging() const { return is_dragging_; }
    [[nodiscard]] bool IsWithinBounds() const { return is_within_bounds_; }

private:
    Camera& camera_;
    GLFWwindow* window_;

    glm::vec2 cue_ball_center_;
    float cue_ball_radius_;
    glm::vec2 aim_position_;
    bool is_dragging_;
    bool is_visible_;
    bool is_highlighted_;
    bool is_within_bounds_;

    // 2D representations
    std::shared_ptr<Object> cue_ball_sprite_;
    std::shared_ptr<Object> red_dot_sprite_; // Red dot for aiming

    // Shader
    std::shared_ptr<Shader> cueBallShader_;
	std::shared_ptr<Shader> redDotShader_;

    void DrawRedDot(const glm::mat4& screenProjection);
    std::shared_ptr<Mesh> CreateCircleMesh(float radius, int segments);
};
