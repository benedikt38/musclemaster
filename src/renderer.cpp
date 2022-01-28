#include "renderer.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::createObjectBuffers(Object object)
{
	if (object.getMeshType() == MeshType::Triangle)
	{
		m_points = object.getTriangleMesh().vertices.size();
		m_indices = object.getTriangleMesh().indices.size();

		glGenBuffers(1, &m_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(Vertex), &object.getTriangleMesh().vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec3), &object.getTriangleMesh().normals[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec2), &object.getTriangleMesh().uvcoords[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_indexlist);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices * sizeof(unsigned int), &object.getTriangleMesh().indices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);

		glBindVertexArray(0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
	else if (object.getMeshType() == MeshType::Quad)
	{
		m_points = object.getQuadMesh().vertices.size();
		m_indices = object.getQuadMesh().indices.size();

		glGenBuffers(1, &m_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec4), &object.getQuadMesh().vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec4), &object.getQuadMesh().normals[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec2), &object.getQuadMesh().uvcoords[0], GL_STATIC_DRAW);

		glGenBuffers(1, &m_indexlist);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices * sizeof(unsigned int), &object.getQuadMesh().indices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);

		glBindVertexArray(0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

}

void Renderer::createMaterialBuffer(std::vector<Material>* materials)
{
	glCreateBuffers(1, &m_ssbo_material_handel);
	glNamedBufferStorage(m_ssbo_material_handel, sizeof(Material) * materials->size(), &materials->at(0), NULL);
}

void Renderer::render(Object object)
{
	glBindVertexArray(m_vao);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_ssbo_material_handel);
	glDrawElements(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, 24);
	//glDrawElements(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, &m_indices);
}
