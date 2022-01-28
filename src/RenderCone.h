#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GL/glew.h>
#include <vector>

class RenderCone
{
public:
	RenderCone(glm::vec3 pos, float baseradius, float apexradius, float height, int resolution, int count);
	void createBuffers();
	void updateBuffers();
	void render(int n);
	void render();
	GLuint getVertexBuffer();
	GLuint getVAO();
	std::vector<glm::vec4> getVertices();
	std::vector<glm::vec3> getnormals();
protected:
	GLuint m_vao;
	GLuint m_vertexbuffer;
	GLuint m_positionBuffer;
	GLuint m_normalbuffer;
	GLuint m_uvbuffer;
	GLuint m_indexlist;
	int m_instanceCount;
	int m_points;
	int m_indices;
	std::vector<glm::vec4> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_uvs;
	std::vector<unsigned int> m_index;
	std::vector<glm::vec3> m_tangents;
private:
	void create();
	glm::vec4 m_pos;
	float m_baseradius;
	float m_apexradius;
	float m_height;
	float m_slope;
	glm::vec3 m_v;
	glm::vec3 m_u;
	glm::vec3 m_w;
	glm::vec3 m_basepoint;
	glm::vec3 m_apexpoint;
	int m_resolution;
};
