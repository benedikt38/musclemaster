#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

class RenderSphere
{
public:
	RenderSphere(glm::vec4 center, float radius, int resolution, int count);
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
	int m_particleCount;
	int m_points;
	int m_indices;
	std::vector<glm::vec4> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec2> m_uvs;
	std::vector<unsigned int> m_index;
	std::vector<glm::vec3> m_tangents; 
private:
	void create(glm::vec4 center, float radius, int resolution);
	glm::vec4 m_Center;
	float m_radius;
	int m_resolution;
};