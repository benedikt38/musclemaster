#pragma once

#include "definitions.h"
#include <filesystem>

enum class MeshType { Triangle, Quad };

struct Halfedge;
struct Face;
struct Vertex;

struct TriangleMesh
{
	std::vector<glm::vec4> normals;
	std::vector<glm::vec4> colors;
	std::vector<Halfedge> halfedges;
	std::vector<Vertex> vertices;
	std::vector<glm::vec2> uvcoords;
	std::vector<Face> faces;
	std::vector<unsigned char*> materials;
	std::vector<unsigned int> indices;
};

struct QuadMesh
{
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec2> uvcoords;
	std::vector<glm::vec4> colors;
	std::vector<glm::ivec4> faces;
	std::vector<unsigned char*> materials;
	std::vector<unsigned int> indices;
};



struct Vertex
{
	glm::vec3 position;
	int outgoingHalfedge;
};

struct Halfedge
{
	int opposite;
	unsigned int next;
	unsigned int face;
	unsigned int vertex;
};

struct Face
{
	int halfedge;
};

struct Material
{
	glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
	float kd = 1.0f;
	float ks = 0.0f;
	float shininess = 15.0f;
	GLuint64 textureAddress = 0;
};

class Object
{
public:
	Object(MeshType meshType, const std::filesystem::path &meshPath);
	Object(MeshType meshType, const std::filesystem::path & meshPath, const std::filesystem::path & texturePath);
	Object(const std::filesystem::path &filePath, int width, int height);
	~Object();
	glm::mat4 getModelMatrix();
	MeshType getMeshType();
	TriangleMesh getTriangleMesh();
	QuadMesh getQuadMesh();
	std::vector<Material>* getMaterials();
	std::vector<Object*> getChildren();
	void addMaterial(const std::filesystem::path &filePath, int width, int height);
private:
	void addChildren(std::vector<Object*> children);
	void setModelMatrix(glm::mat4 modelMatrix);
	void render();
	TriangleMesh m_triangleMesh;
	QuadMesh m_quadMesh;
	MeshType m_meshType;
	std::vector<Material> m_materials;
	glm::mat4 m_modelMatrix;
	std::vector<Object*> m_children;
};