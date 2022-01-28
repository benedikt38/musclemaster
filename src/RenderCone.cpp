#include "RenderCone.h"

RenderCone::RenderCone(glm::vec3 pos, float baseradius, float apexradius, float height, int resolution, int count)
{
	m_baseradius = baseradius;
	m_apexradius = apexradius;
	m_height = height;
	m_basepoint = pos;
	m_apexpoint = pos + glm::vec3(0.f, height, 0.f);
	m_resolution = resolution;
	create();
}

void RenderCone::createBuffers()
{
	m_points = m_vertices.size();
	m_indices = m_index.size();

	glGenBuffers(1, &m_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec4), &m_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec3), &m_normals[0], GL_STATIC_DRAW);

	/*glGenBuffers(1, &m_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, m_points * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);*/

	glGenBuffers(1, &m_indexlist);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices * sizeof(unsigned int), &m_index[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalbuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/*glBindBuffer(GL_ARRAY_BUFFER, m_uvbuffer);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexlist);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void RenderCone::updateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_instanceCount * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

}

void RenderCone::render(int n)
{
	int last_vao = 0;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
	glBindVertexArray(m_vao);
	glDrawElementsInstanced(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, 0, n);
	glBindVertexArray(last_vao);
}

void RenderCone::render()
{

	glBindVertexArray(m_vao);
	glPointSize(5.0f);
	glDrawElements(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, 0);
}

GLuint RenderCone::getVertexBuffer()
{
	return m_vertexbuffer;
}

GLuint RenderCone::getVAO()
{
	return m_vao;
}

std::vector<glm::vec4> RenderCone::getVertices()
{
	return m_vertices;
}

std::vector<glm::vec3> RenderCone::getnormals()
{
	return m_normals;
}

void RenderCone::create()
{
	//generate points
	for (int i = 0; i < m_resolution; i++)  //radius
	{
		float k = i / static_cast<float>(m_resolution);
		float phi = 2 * glm::pi<float>() * k;
		m_vertices.push_back(glm::vec4(m_basepoint, 1.0) + m_baseradius * glm::vec4(glm::sin(phi), 0.0, glm::cos(phi), 0.0));
		m_vertices.push_back(glm::vec4(m_apexpoint, 1.0) + m_apexradius * glm::vec4(glm::sin(phi), 0.0, glm::cos(phi), 0.0));
		m_normals.push_back(normalize((m_basepoint + m_baseradius * glm::vec3(glm::sin(phi), 0.0, glm::cos(phi)))));
		m_normals.push_back(normalize((m_basepoint + m_apexradius * glm::vec3(glm::sin(phi), 0.0, glm::cos(phi)))));
	}

	m_points = m_vertices.size();

	// create index list
	auto id_res = 2 * m_resolution;
	//triangulate side
	for (int i = 0; i <= id_res; i++)
	{
		m_index.push_back(i % id_res);
		m_index.push_back((i + 1) % id_res);
		m_index.push_back((i + 2) % id_res);
	}
	//triangulate bottom
	for (int i = 1; i <= m_resolution - 1; i++)
	{
		m_index.push_back(0);
		m_index.push_back(i * 2);
		m_index.push_back(i * 2 + 2);
	}
	//triangulate top
	for (int i = 1; i < m_resolution - 1; i++)
	{
		m_index.push_back(1);
		m_index.push_back(i * 2 + 1);
		m_index.push_back(i * 2 + 3);
	}
	m_indices = m_index.size();

	createBuffers();

	//m_bufferInit = true;
}
