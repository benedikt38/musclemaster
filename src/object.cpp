#include "object.h"
#include "fileLoader.h"
#include "renderer.h"

Object::Object(MeshType meshType, const std::filesystem::path &meshPath)
{
	m_modelMatrix = glm::mat4(1.0f);
	switch (meshType)
	{
	case MeshType::Triangle:
		m_triangleMesh = FileLoader::triangleMeshLoader(meshPath);
		m_meshType = MeshType::Triangle;
	break;
	case MeshType::Quad:
		m_quadMesh = FileLoader::quadMeshLoader(meshPath);
		m_meshType = MeshType::Quad;
	break;
	}
}

Object::Object(MeshType meshType, const std::filesystem::path &meshPath, const std::filesystem::path &texturePath)
{
	m_modelMatrix = glm::mat4(1.0f);
	switch (meshType)
	{
	case MeshType::Triangle:
		m_triangleMesh = FileLoader::triangleMeshLoader(meshPath);
		m_meshType = MeshType::Triangle;
		break;
	case MeshType::Quad:
		m_quadMesh = FileLoader::quadMeshLoader(meshPath);
		m_meshType = MeshType::Quad;
		break;
	}
}

Object::Object(const std::filesystem::path &filePath, int width, int height)
{
	m_materials.push_back(FileLoader::loadImage(filePath, width, height));
}

Object::~Object()
{
}

glm::mat4 Object::getModelMatrix()
{
	return m_modelMatrix;
}

MeshType Object::getMeshType()
{
	return m_meshType;
}

TriangleMesh Object::getTriangleMesh()
{
	return m_triangleMesh;
}

QuadMesh Object::getQuadMesh()
{
	return m_quadMesh;
}

std::vector<Material>* Object::getMaterials()
{
	return &m_materials;
}

std::vector<Object*> Object::getChildren()
{
	return m_children;
}

void Object::addMaterial(const std::filesystem::path & filePath, int width, int height)
{
	m_materials.push_back(FileLoader::loadImage(filePath, width, height));
}

void Object::addChildren(std::vector<Object*> children)
{
	for (int i = 0; i < children.size(); i++)
	{
		m_children.push_back(children.at(i));
	}
}

void Object::setModelMatrix(glm::mat4 modelMatrix)
{
	m_modelMatrix = modelMatrix;
}

void Object::render()
{
	Renderer renderer;
	renderer.createObjectBuffers(*this);
}
