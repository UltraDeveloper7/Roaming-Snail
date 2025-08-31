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
    ~TextRenderer(); // cleanup FT resources

    void Init();
    void UpdateProjectionMatrix(int width, int height);
    void Update() const;
    void Render(std::vector<Text>& texts);

private:
    // --- UTF-8 aware rendering helpers ---
    // decode next UTF-8 codepoint; advances i; returns U+FFFD on error
    uint32_t NextCodepoint(const std::string& s, size_t& i) const;

    // ensure glyph exists in 'characters_' (lazy load); false if FT load fails
    bool EnsureGlyph(uint32_t cp);

    // render a single codepoint (assumes EnsureGlyph)
    void RenderGlyph(float& x, float& y, uint32_t cp);

    float CalculateTextWidth(const std::string& text);
    void  Load();

    // Load an extra FT_Face and add to fallback list (primary inserted first)
    bool AddFaceFromPath(const std::filesystem::path& p);

private:
    // glyph cache by Unicode codepoint
    std::unordered_map<uint32_t, Character> characters_{};

    std::unique_ptr<Shader> text_shader_ = nullptr;

    unsigned vao_{}, vbo_{};
    glm::vec2 font_scale_{ 1.0f };
    glm::mat4 projection_matrix_{};

    // FreeType
    FT_Library ft_{};
    // faces_[0] is the primary UI face; the rest are fallbacks
    std::vector<FT_Face> faces_{};
    std::vector<std::filesystem::path> face_paths_{};

    // --- styling ---
    bool      draw_shadow_ = true;
    glm::vec2 shadow_px_ = { 3.0f, -3.0f }; // offset in pixels (y is up)
    glm::vec3 shadow_color_ = { 0.0f, 0.0f, 0.0f };
    float     shadow_alpha_ = 0.7f;
};

