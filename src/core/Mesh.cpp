#include "../precompiled.h"
#include "Mesh.hpp"
#include "Vertex.hpp"

// Make sure the Vertex layout matches what shaders expect.
static_assert(offsetof(Vertex, position) == 0, "Vertex.position must be at offset 0");


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices, const int material_id) : vao_{}, ebo_{}, vbo_{},
	vertex_count_(vertices.size()),
	index_count_(indices.size()),
	material_id_(material_id)
{
	// Create VAO first: it will capture VBO/EBO bindings & attrib setup.
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	// Vertex buffer
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertex_count_ * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// Optional index buffer (keep it bound while VAO is bound)
	if (index_count_ > 0)
	{
		glGenBuffers(1, &ebo_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count_ * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	}

	// Describe the vertex layout once per VAO
	setupVertexFormat();

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh::~Mesh()
{
	Clear();
}

void Mesh::setupVertexFormat() const
{
	// Position (location = 0)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<const void*>(offsetof(Vertex, position)));

	// Normal (location = 1)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<const void*>(offsetof(Vertex, normal)));

	// UV (location = 2)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex),
		reinterpret_cast<const void*>(offsetof(Vertex, uv)));
}

void Mesh::Bind() const
{
    glBindVertexArray(vao_);
}

void Mesh::Unbind() const
{
    glBindVertexArray(0);
}

void Mesh::Draw() const
{
	if (index_count_ > 0) {
		glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);
	}
	else {
		glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
	}
}

void Mesh::Clear()
{
	if (vbo_) { glDeleteBuffers(1, &vbo_); vbo_ = 0; }
	if (ebo_) { glDeleteBuffers(1, &ebo_); ebo_ = 0; }
	if (vao_) { glDeleteVertexArrays(1, &vao_); vao_ = 0; }
}