#include "../precompiled.h"
#include "TextRenderer.hpp"

TextRenderer::TextRenderer() : text_shader_(std::make_unique<Shader>(Config::text_vertex_path, Config::text_fragment_path)), vao_{}, vbo_{}
{
	Load();

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void TextRenderer::Init()
{
	projection_matrix_ = glm::ortho(0.0f, static_cast<float>(Config::width), 0.0f, static_cast<float>(Config::height));
	Update();
}

void TextRenderer::UpdateProjectionMatrix(const int width, const int height)
{
	projection_matrix_ = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
	font_scale_ = glm::vec2(width / static_cast<float>(Config::width), height / static_cast<float>(Config::height));
}

void TextRenderer::Update() const
{
	text_shader_->Bind();
	text_shader_->SetMat4(projection_matrix_, "projectionMatrix");
}

void TextRenderer::Render(std::vector<Text>& texts)
{

	text_shader_->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao_);

	for (auto& [position_x, position_y, text, scale, alignment, selected] : texts)
	{
		// pick color (red when selected, white otherwise)
		glm::vec3 color = selected ? glm::vec3(1.0f, 0.0f, 0.0f)
			: glm::vec3(1.0f);
		text_shader_->SetVec3(color, "textColor");

		const glm::vec2 saved_scale = font_scale_;
		font_scale_ *= scale;

		// alignment in pixels
		float width_px = CalculateTextWidth(text); // uses current font_scale_
		if (alignment == Alignment::CENTER)       position_x -= width_px * 0.5f;
		else if (alignment == Alignment::RIGHT)   position_x -= width_px;

		// optional drop shadow (draw first, slightly offset)
		if (draw_shadow_) {
			glm::vec3 shadow = shadow_color_;
			// push a darker color; your text shader uses only RGB (alpha via texture)
			text_shader_->SetVec3(shadow, "textColor");

			// lift the shadow a tad when selected/hovered
			const glm::vec2 lift = selected ? glm::vec2(3.0f, -3.0f) : shadow_px_;

			float sx = position_x + lift.x;
			float sy = position_y + lift.y;
			for (unsigned char c : std::string_view(text)) {
				RenderCharacter(sx, sy, c);
			}
			// restore text color
			text_shader_->SetVec3(color, "textColor");
		}

		// main pass
		for (unsigned char c : std::string_view(text)) {
			RenderCharacter(position_x, position_y, c);
		}

		font_scale_ = saved_scale;
	}

	texts.clear();

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void TextRenderer::RenderCharacter(float& x, float& y, const unsigned char ch)
{
	const Character& chd = characters_.at(ch); // throws if missing → easy to spot bad glyphs

	const float position_x = x + chd.bearing.x * font_scale_.x;
	const float position_y = y - (chd.size.y - chd.bearing.y) * font_scale_.y;

	const float w = chd.size.x * font_scale_.x;
	const float h = chd.size.y * font_scale_.y;

	const float vertices[6][4] =
	{
		{ position_x,     position_y + h,   0.0f, 0.0f },
		{ position_x,     position_y,       0.0f, 1.0f },
		{ position_x + w, position_y,       1.0f, 1.0f },

		{ position_x,     position_y + h,   0.0f, 0.0f },
		{ position_x + w, position_y,       1.0f, 1.0f },
		{ position_x + w, position_y + h,   1.0f, 0.0f }
	};

	if (chd.texture) chd.texture->Bind();

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	x += (chd.advance >> 6) * font_scale_.x;
}

float TextRenderer::CalculateTextWidth(const std::string& text)
{
	float width = 0.0f;
	for (unsigned char c : std::string_view(text)) {
		auto it = characters_.find(c);
		if (it != characters_.end())
			width += (it->second.advance >> 6) * font_scale_.x;
	}
	return width;
}


void TextRenderer::Load()
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		[[unlikely]] throw std::exception("Could not init FreeType Library");

	const auto font_path = std::filesystem::current_path() / "assets/fonts" / Config::font_path;

	FT_Face face;
	if (FT_New_Face(ft, font_path.string().c_str(), 0, &face))
		[[unlikely]] throw std::exception("Failed to load font");

	FT_Set_Pixel_Sizes(face, 0, Config::default_font_size);
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load first 96 ASCII characters
	for (int i = 32; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
			[[unlikely]] throw std::exception(std::string("Failed to load glyph \'" + i + '\'').c_str());

		auto character = Character(
			std::make_unique<Texture>(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, 1),
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned>(face->glyph->advance.x));

		characters_.insert(std::pair(i, std::move(character)));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}