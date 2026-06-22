#pragma once

class Shader
{
public:
	// Compiles and links a shader program from files under src/shaders.
	Shader(const std::string& vertex_path, const std::string& fragment_path, const std::string& geometry_path = {});
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&&) = delete;
	Shader& operator=(Shader&&) = delete;

	// Makes this program the active OpenGL shader.
	void Bind() const;
	void Unbind() const;

	// Uniform helpers. They hide glUniform calls and cache uniform locations.
	void SetVec2(const glm::vec2& v, const std::string& name) const;
	void SetVec3(const glm::vec3& v, const std::string& name) const;
	void SetVec4(const glm::vec4& v, const std::string& name) const;
	void SetMat4(const glm::mat4& m, const std::string& name) const;
	void SetFloat(float s, const std::string& name) const;
	void SetInt(int n, const std::string& name) const;
	void SetIntArray(const char* name, const int* values, int count);
	void SetBool(bool c, const std::string& name) const;

	// Restores common material/rendering switches before each draw path.
	void ResetRenderFlags() const;
	// Restores depth-pass switches so shadow drawing starts from a known state.
	void ResetDepthFlags() const;

	// Exposes the raw OpenGL id for special cases such as direct binding/debug.
	unsigned GetID() const {
		return id_;
	}

private:
	// File loading, compilation, and program link stages.
	[[nodiscard]] std::string LoadShaderSource(const std::string& path) const;
	[[nodiscard]] unsigned LoadShader(unsigned type, const std::string& path) const;
	void LinkProgram(unsigned vertex, unsigned fragment, unsigned geometry = 0);

	GLint GetUniformLocation(const std::string& name) const;

	// OpenGL program id.
	unsigned id_;
	// Uniform location cache avoids repeated glGetUniformLocation calls.
	mutable std::unordered_map<std::string, GLint> uniform_cache_;
};
