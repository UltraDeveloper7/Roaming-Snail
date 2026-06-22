#pragma once
#include "../precompiled.h"
#include "../Logger.hpp"
#include "Menu.hpp"
#include "../core/Shader.hpp"
#include "../core/Texture.hpp"


struct Character
{
    // One glyph is stored as a small OpenGL texture plus FreeType metrics.
	std::unique_ptr<Texture> texture = nullptr;
	glm::ivec2 size{};
	glm::ivec2 bearing{};
	unsigned advance{};
};

class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer();

    // Initializes FreeType, loads font faces, creates shader and glyph buffers.
    void Init();
    // Rebuilds orthographic projection for screen-space text.
    void UpdateProjectionMatrix(int width, int height);
    // Uploads projection/uniforms that may change per frame.
    void Update() const;
    // Renders the queued Text commands from Menu/App.
    void Render(std::vector<Text>& texts);

private:
    // UTF-8 aware rendering helpers.
    // Decodes the next codepoint, advances i, and returns U+FFFD on error.
    uint32_t NextCodepoint(const std::string& s, size_t& i) const;

    // Lazily loads a glyph into characters_. Returns false if FreeType fails.
    bool EnsureGlyph(uint32_t cp);

    // Draws one loaded glyph and advances the pen position.
    void RenderGlyph(float& x, float& y, uint32_t cp);

    // Used by menu alignment and button hit boxes.
    float CalculateTextWidth(const std::string& text);
    // Loads primary and fallback font faces.
    void  Load();

    // Loads an extra FT_Face and adds it to the fallback list.
    bool AddFaceFromPath(const std::filesystem::path& p);

private:
    // Glyph cache by Unicode codepoint.
    std::unordered_map<uint32_t, Character> characters_{};

    std::unique_ptr<Shader> text_shader_ = nullptr;

    // Quad geometry reused for every glyph.
    unsigned vao_{}, vbo_{};
    glm::vec2 font_scale_{ 1.0f };
    glm::mat4 projection_matrix_{};

    // FreeType library and loaded font faces. faces_[0] is the primary UI face.
    FT_Library ft_{};
    std::vector<FT_Face> faces_{};
    std::vector<std::filesystem::path> face_paths_{};

    // Text styling: shadow improves readability over bright terrain.
    bool      draw_shadow_ = true;
    glm::vec2 shadow_px_ = { 3.0f, -3.0f };
    glm::vec3 shadow_color_ = { 0.0f, 0.0f, 0.0f };
    float     shadow_alpha_ = 0.7f;
};

