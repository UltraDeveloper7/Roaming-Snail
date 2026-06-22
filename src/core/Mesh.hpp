#pragma once
#include "../precompiled.h"
#include "Vertex.hpp"

class Mesh
{
public:
	// Uploads vertex/index data to GPU buffers and records the material slot
	// used when Object draws this mesh.
	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices = {}, int material_id = 0);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator= (const Mesh&) = delete;
	Mesh& operator= (Mesh&&) = delete;

	// Bind/unbind the VAO before drawing.
	void Bind() const;
	void Unbind() const;
	// Issues glDrawElements when indices exist, otherwise glDrawArrays.
	void Draw() const;
	// Frees OpenGL resources owned by this mesh.
	void Clear();
	[[nodiscard]] int GetMaterialId() const { return material_id_; }

private:
    // OpenGL objects that store vertex layout and buffers.
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;

    // Draw counts.
    GLsizei vertex_count_  = 0;
    GLsizei index_count_   = 0;

    // Index into the owning Object's material array.
    int material_id_ = 0;

    // Defines position, normal, uv, and material-id vertex attributes.
    void setupVertexFormat() const;
};
