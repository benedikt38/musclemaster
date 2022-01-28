#pragma once

#include "definitions.h"
#include "fileLoader.h"

class Renderer
{
public:
	Renderer();
	~Renderer();
	void createObjectBuffers(Object object);
	void createMaterialBuffer(std::vector<Material>* materials);
	void render(Object object);
private:
	GLuint m_vao;
	GLuint m_tex;
	GLuint m_ssbo_material_handel;
	GLuint m_vertexbuffer;
	GLuint m_positionBuffer;
	GLuint m_normalbuffer;
	GLuint m_uvbuffer;
	GLuint m_indexlist;
	int m_points;
	int m_indices;
};
