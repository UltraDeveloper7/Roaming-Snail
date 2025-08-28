#include "../precompiled.h"
#include "CueBallMap.hpp"
#include "../core/Loader.hpp"

CueBallMap::CueBallMap(Camera& camera, GLFWwindow* window)
    : camera_(camera), window_(window), is_dragging_(false), is_visible_(false), is_highlighted_(false), is_within_bounds_(false) {
    int window_width, window_height;
    glfwGetFramebufferSize(window_, &window_width, &window_height);

    cue_ball_radius_ = 60.0f;  // Adjust as needed for 1920x1080
    cue_ball_center_ = glm::vec2(window_width - cue_ball_radius_ - 10.0f, window_height - cue_ball_radius_ - 10.0f);
    aim_position_ = cue_ball_center_;  // Ensure the red dot starts in the center of the cue ball

    // Load the cue ball texture
    auto cue_ball_texture_path = (std::filesystem::current_path() / "assets/textures/ball0.jpg").string();
    auto cue_ball_texture = Loader::LoadTexture(cue_ball_texture_path);

    // Create material for the cue ball
    auto cue_ball_material = std::make_shared<Material>();
    cue_ball_material->diffuse_texture = cue_ball_texture;

    // Create a circular mesh for the cue ball
    auto cue_ball_mesh = CreateCircleMesh(cue_ball_radius_ * 0.00050f, 64);

    // Create the cue ball object
    cue_ball_sprite_ = std::make_shared<Object>();
    cue_ball_sprite_->SetMaterials({ cue_ball_material });
    cue_ball_sprite_->SetMeshes({ cue_ball_mesh });
    cue_ball_sprite_->translation_ = glm::vec3(cue_ball_center_, 0.0f);
    cue_ball_sprite_->scale_ = glm::vec3(cue_ball_radius_);

    // Create a red dot texture
    unsigned char red_pixel[3] = { 136, 8, 8 };
    auto red_texture = std::make_shared<Texture>(red_pixel, 1, 1, 3);

    // Create material for the red dot
    auto red_material = std::make_shared<Material>();
    red_material->diffuse_texture = red_texture;

    // Create a circular mesh for the red dot
    auto red_dot_mesh = CreateCircleMesh(0.1f, 64);

    // Create the red dot object
    red_dot_sprite_ = std::make_shared<Object>();
    red_dot_sprite_->SetMaterials({ red_material });
    red_dot_sprite_->SetMeshes({ red_dot_mesh });

    // Load the cue ball shader
    cueBallShader_ = std::make_shared<Shader>(
        Config::CueMap_vertex_path,
        Config::CueMap_fragment_path
    );

    // Set the material.diffuse_texture sampler to texture unit 0 for cue ball shader
    cueBallShader_->Bind();
    cueBallShader_->SetInt(4, "material.diffuse_texture");
    cueBallShader_->SetBool(true, "material.hasDiffuseMap");
    cueBallShader_->Unbind();

    // Load the red dot shader
    redDotShader_ = std::make_shared<Shader>(
        Config::sprite_vertex_path,
        Config::sprite_fragment_path
    );

    // Set the material.diffuse_texture sampler to texture unit 0 for red dot shader
    redDotShader_->Bind();
    redDotShader_->SetInt(0, "material.diffuse_texture");
    redDotShader_->SetBool(true, "material.hasDiffuseMap");
    redDotShader_->Unbind();

    // Initialize window size and position
    UpdateWindowSize();
}

std::shared_ptr<Mesh> CueBallMap::CreateCircleMesh(float radius, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;

    // Center vertex
    vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.5f}, 0 });

    // Circle vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = glm::two_pi<float>() * i / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back({ {x, y, 0.0f}, {0.0f, 0.0f, 1.0f}, {x * 0.5f + 0.5f, y * 0.5f + 0.5f}, 0 });
    }

    // Indices
    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    return std::make_shared<Mesh>(vertices, indices);
}

void CueBallMap::UpdateWindowSize() {
    int window_width, window_height;
    if (!window_) return;

    glfwGetFramebufferSize(window_, &window_width, &window_height);

    // Adjust the scale of the cue ball relative to the window size
    float scale_factor = static_cast<float>(window_width) / 1920.0f;
    cue_ball_radius_ = 60.0f * scale_factor;
    cue_ball_sprite_->scale_ = glm::vec3(cue_ball_radius_);

    // Calculate the new center of the cue ball
    glm::vec2 new_cue_ball_center = glm::vec2(window_width - cue_ball_radius_ - 10.0f, window_height - cue_ball_radius_ - 10.0f);

    // Calculate the offset of the red dot from the center of the cue ball
    glm::vec2 offset = aim_position_ - cue_ball_center_;

    // Update the center of the cue ball
    cue_ball_center_ = new_cue_ball_center;

    // Update the aim position relative to the new center of the cue ball
    aim_position_ = cue_ball_center_ + offset;
}


void CueBallMap::Draw() {
    if (!is_visible_) return; // Only render if visible

    UpdateWindowSize();

    // Render the 2D projection to the screen
    int window_width, window_height;
    glfwGetFramebufferSize(window_, &window_width, &window_height);

    // Set orthographic projection covering the full screen
    glm::mat4 screenProjection = glm::ortho(
        0.0f, static_cast<float>(window_width),
        0.0f, static_cast<float>(window_height)
    );

    // Position and scale the 2D projection in the top-right corner with padding
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(cue_ball_center_, 0.0f));
    model = glm::scale(model, glm::vec3(cue_ball_radius_ * 25.0f, cue_ball_radius_ * 25.0f, 1.0f));

    // Disable depth testing to ensure the 2D projection is rendered on top
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind the cue ball texture
    cue_ball_sprite_->GetMaterials()[0]->diffuse_texture->Bind();

    // Draw the 2D projection using cue ball shader
    cueBallShader_->Bind();
    cueBallShader_->SetMat4(screenProjection, "projection");
    cueBallShader_->SetMat4(model, "model");

    cue_ball_sprite_->Draw(cueBallShader_);

    // Draw red dot in the center of the screen
    DrawRedDot(screenProjection);

    cueBallShader_->Unbind();

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

void CueBallMap::DrawRedDot(const glm::mat4& screenProjection) {
    // Position and scale the red dot in the center of the screen
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(aim_position_, 0.0f));
    model = glm::scale(model, glm::vec3(cue_ball_radius_ * 1.5f)); // Adjust size as needed

    // Bind the red dot texture
    red_dot_sprite_->GetMaterials()[0]->diffuse_texture->Bind();

    // Draw the red dot using red dot shader
    redDotShader_->Bind();
    redDotShader_->SetMat4(screenProjection, "projection");
    redDotShader_->SetMat4(model, "model");

    red_dot_sprite_->Draw(redDotShader_);

    redDotShader_->Unbind();
}

void CueBallMap::HandleMouseInput(GLFWwindow* window) {
    if (!is_visible_) return;

    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    int window_width, window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);

    // Flip Y so (0,0) is bottom-left in window coords
    mouse_y = window_height - mouse_y;

    // Transform to 2D projection-local coords
    float local_x = static_cast<float>(mouse_x) - cue_ball_center_.x;
    float local_y = static_cast<float>(mouse_y) - cue_ball_center_.y;

    glm::vec2 local_mouse_pos(local_x, local_y);

    // Check if the mouse is within the bounds of the white ball circle
    is_within_bounds_ = glm::distance(local_mouse_pos, glm::vec2(0.0f, 0.0f)) <= cue_ball_radius_;

    if (is_within_bounds_) {
        glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR)); // Change cursor to hand
        camera_.SetTopDownView(true); // Ensure the camera is in top-down view
    }
    else {
        glfwSetCursor(window, glfwCreateStandardCursor(GLFW_ARROW_CURSOR)); // Change cursor back to arrow
    }

    int left_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    if (left_button_state == GLFW_PRESS) {
        // Prevent the camera from switching views if in top-down view
        if (camera_.IsTopDownView()) {
            // Start drag if within 'hit' range of the red dot
            if (!is_dragging_ && glm::distance(local_mouse_pos, aim_position_ - cue_ball_center_) <= 10.0f) {
                is_dragging_ = true;
            }

            // Move the red dot, clamped to cue_ball_radius_ - 26.0f
            if (is_dragging_) {
                glm::vec2 direction = local_mouse_pos;
                float dist = glm::length(direction);
                float clamped_radius = cue_ball_radius_ - 26.0f; // Slightly reduce the radius for stricter clamping
                if (dist > clamped_radius) {
                    direction = glm::normalize(direction) * clamped_radius;
                }
                aim_position_ = cue_ball_center_ + direction;

            }
        }
    }
    else {
        is_dragging_ = false;
    }
}


glm::vec2 CueBallMap::GetSpin() const
{
    // Offset from cue-ball center to the red dot (in screen pixels, bottom-left origin)
    glm::vec2 offset = aim_position_ - cue_ball_center_;

    //  - offset.y > 0 (dot above center)  => topspin   (spin.y > 0)
    //  - offset.y < 0 (dot below center)  => backspin   (spin.y < 0)
    //  - offset.x > 0 (dot right of center)=> right english (spin.x > 0)
    //  - offset.x < 0 (dot left of center) => left english  (spin.x < 0)
    glm::vec2 spin = offset / cue_ball_radius_;

    // Clamp to [-1,1] for stability
    spin.x = glm::clamp(spin.x, -1.0f, 1.0f);
    spin.y = glm::clamp(spin.y, -1.0f, 1.0f);

    return spin;
}

void CueBallMap::ResetAim() {
    aim_position_ = cue_ball_center_;
}

void CueBallMap::SetVisible(bool visible) {
    is_visible_ = visible;
}
