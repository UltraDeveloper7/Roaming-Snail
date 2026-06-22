#pragma once

class Texture
{
public:
	// Allocates an empty texture, usually for render targets or generated maps.
	Texture(int size, bool mipmap);
	// Uploads an LDR image loaded by stb_image.
	Texture(unsigned char* image_data, int width, int height, int channels);
	// Uploads an HDR floating-point image for environment lighting.
	Texture(float* image_data, int width, int height);
	~Texture();

	Texture(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture& operator= (const Texture&) = delete;
	Texture& operator= (Texture&&) = delete;

	// Binds the texture to its recorded OpenGL target.
	void Bind() const;
	[[nodiscard]] int GetId() const { return static_cast<int>(texture_); }

private:
	// OpenGL texture handle and target type (2D, cubemap, etc.).
	GLuint texture_;
	GLenum type_;
};
