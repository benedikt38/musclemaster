#include "RenderSphere.h"

RenderSphere::RenderSphere(glm::vec4 center, float radius, int resolution, int count)
{
	m_radius = radius;
	m_resolution = resolution;
	m_Center = center;
	m_particleCount = count;
	create(m_Center, m_radius, m_resolution);
}

void RenderSphere::createBuffers()
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

void RenderSphere::updateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_particleCount * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

}

void RenderSphere::render(int n)
{
	glBindVertexArray(m_vao);
	glDrawElementsInstanced(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, 0, n);
}

void RenderSphere::render()
{
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, 0);
}

GLuint RenderSphere::getVertexBuffer()
{
	return m_vertexbuffer;
}

GLuint RenderSphere::getVAO()
{
	return m_vao;
}

std::vector<glm::vec4> RenderSphere::getVertices()
{
	return m_vertices;
}

std::vector<glm::vec3> RenderSphere::getnormals()
{
	return m_normals;
}

void RenderSphere::create(glm::vec4 center, float radius, int resolution)
{
	float u, v;
	float phi, theta;
	float x, y, z;
	int offset = 0, i, j;
	for (j = 0; j <= resolution; j++)
		for (i = 0; i <= resolution; i++)
		{
			u = i / (float)resolution;
			phi = 2 * glm::pi<float>() * u;
			v = j / (float)resolution;
			theta = glm::pi<float>() * v;
			x = m_Center.x + radius * sin(theta) * sin(phi);
			y = m_Center.y + radius * cos(theta);
			z = m_Center.z + radius * sin(theta) * cos(phi);
			m_vertices.push_back(glm::vec4(x, y, z, 1.0f));
			m_normals.push_back((glm::vec3(x, y, z) - glm::vec3(m_Center)) / radius);
			m_uvs.push_back(glm::vec2(u, 1 - v));
		}
	m_points = m_vertices.size();
	for (j = 0; j < resolution; j++)
	{
		for (i = 0; i < resolution; i++)
		{
			// 1. Triangle
			m_index.push_back(offset + i);
			m_index.push_back(offset + i + resolution + 1);
			m_index.push_back(offset + i + resolution + 1 + 1);
			// 2. Triangle
			m_index.push_back(offset + i + resolution + 1 + 1);
			m_index.push_back(offset + i + 1);
			m_index.push_back(offset + i);
		}
		offset += resolution + 1;
	}
	m_indices = m_index.size();
	createBuffers();
	//m_bufferInit = true;
}
