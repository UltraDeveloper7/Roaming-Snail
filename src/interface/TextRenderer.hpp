#pragma once
#include "../precompiled.h"
#include "Menu.hpp"
#include "../core/Shader.hpp"
#include "../core/Texture.hpp"

struct Character
{
	std::unique_ptr<Texture> texture = nullptr;
	glm::ivec2 size{};
	glm::ivec2 bearing{};
	unsigned advance{};
};

class TextRenderer
{
public:
	TextRenderer();
	void Init();
	void UpdateProjectionMatrix(int width, int height);
	void Update() const;
	void Render(std::vector<Text>& texts);

private:
	void RenderCharacter(float& x, float& y, unsigned char character);
	float CalculateTextWidth(const std::string& text);
	void Load();

	std::unordered_map<int, Character> characters_{};
	std::unique_ptr<Shader> text_shader_ = nullptr;

	unsigned vao_, vbo_;
	glm::vec2 font_scale_{ 1.0f };
	glm::mat4 projection_matrix_{};

	// --- styling ---
	bool      draw_shadow_ = true;
	glm::vec2 shadow_px_ = { 3.0f, -3.0f };    // offset in pixels (y is up)
	glm::vec3 shadow_color_ = { 0.0f, 0.0f, 0.0f };
	float     shadow_alpha_ = 0.7f;
};

