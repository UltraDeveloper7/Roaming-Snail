#pragma once
#include "../precompiled.h"
#include "Vertex.hpp"

class Mesh
{
public:
	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices = {}, int material_id = 0);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator= (const Mesh&) = delete;
	Mesh& operator= (Mesh&&) = delete;

	void Bind() const;
	void Unbind() const;
	void Draw() const;
	void Clear();
	[[nodiscard]] int GetMaterialId() const { return material_id_; }

private:
    // GL objects
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;

    // counts
    GLsizei vertex_count_  = 0;   // number of vertices
    GLsizei index_count_   = 0;   // number of indices

    // meta
    int material_id_ = 0;

    // helpers
    void setupVertexFormat() const;
};
