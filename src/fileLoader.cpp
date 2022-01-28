#include "fileLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include<stbImage\stb_image.h>

FileLoader::FileLoader() 
{

}

TriangleMesh FileLoader::triangleMeshLoader(const std::filesystem::path &path)
{
	if (path.extension() == ".obj" || path.extension() == ".off")
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.string(), aiProcess_JoinIdenticalVertices|aiProcess_Triangulate|aiProcess_GenNormals|aiProcess_GenUVCoords);
		for (int k = 0; k < static_cast<int>(scene->mNumMeshes); k++)
		{
			aiMesh* mesh = scene->mMeshes[k];
			TriangleMesh triMesh;

			if (scene->HasMaterials())
			{
				/*	for (int i = 0; i < scene->mNumMaterials; i++)
					{
						triMesh.materials.at(i) = (unsigned char *)(scene->mMaterials[i]->GetTexture());
					}*/
			}
			triMesh.halfedges.resize(mesh->mNumFaces * 3);
			triMesh.vertices.resize(mesh->mNumVertices);
			triMesh.faces.resize(mesh->mNumFaces);
			triMesh.indices.resize(mesh->mNumFaces * 3);
			triMesh.normals.resize(mesh->mNumVertices);
			triMesh.uvcoords.resize(mesh->mNumVertices);
			triMesh.colors.resize(mesh->mNumVertices);

			#pragma omp parallel for
			for (int i = 0; i < static_cast<int>(mesh->mNumVertices); i++)
			{
				triMesh.vertices.at(i).position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
				triMesh.vertices.at(i).outgoingHalfedge = -1;
			}

			#pragma omp parallel for
			for (int i = 0; i < static_cast<int>(mesh->mNumFaces); i++)
			{
				triMesh.indices.at(i * 3) = mesh->mFaces[i].mIndices[0];
				triMesh.indices.at(i * 3 + 1) = mesh->mFaces[i].mIndices[1];
				triMesh.indices.at(i * 3 + 2) = mesh->mFaces[i].mIndices[2];
			}

			GLuint halfedgeBuffer;
			glCreateBuffers(1, &halfedgeBuffer);
			glNamedBufferStorage(halfedgeBuffer, triMesh.halfedges.size() * sizeof(Halfedge), &triMesh.halfedges[0], GL_DYNAMIC_STORAGE_BIT);

			GLuint vertexBuffer;
			glCreateBuffers(1, &vertexBuffer);
			glNamedBufferStorage(vertexBuffer, triMesh.vertices.size() * sizeof(Vertex), &triMesh.vertices[0], GL_DYNAMIC_STORAGE_BIT);

			GLuint indexBuffer;
			glCreateBuffers(1, &indexBuffer);
			glNamedBufferStorage(indexBuffer, triMesh.indices.size() * sizeof(int), &triMesh.indices[0], GL_DYNAMIC_STORAGE_BIT);

			ShaderProgram shaderProgram(SHADERS_PATH "/generateHalfedges.comp");
			shaderProgram.use();
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, halfedgeBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexBuffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, indexBuffer);
			glDispatchCompute(static_cast<int>(glm::ceil(triMesh.faces.size() / 64.f)), 1, 1);

			glGetNamedBufferSubData(vertexBuffer, 0, triMesh.vertices.size() * sizeof(Vertex), &triMesh.vertices[0]);

			/*ShaderProgram shaderProgram2(SHADERS_PATH "/generateOpposite.comp");
			shaderProgram2.use();
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, halfedgeBuffer);
			glDispatchCompute(glm::ceil(triMesh.halfedges.size()/64.f), 1, 1);

			glGetNamedBufferSubData(halfedgeBuffer, 0, triMesh.halfedges.size() * sizeof(Halfedge), &triMesh.halfedges[0]);*/

			#pragma omp parallel for
			for (int i = 0; i < static_cast<int>(mesh->mNumVertices); i++)
			{
				triMesh.normals.at(i) = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 1.0f);
				if (mesh->HasTextureCoords(0))
				{
					triMesh.uvcoords.at(i) = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
					int mindex = mesh->mMaterialIndex;
				}
				if (mesh->HasVertexColors(0))
				{
					triMesh.colors.at(i) = glm::vec4(mesh->mColors[i]->r, mesh->mColors[i]->g, mesh->mColors[i]->b, mesh->mColors[i]->a);
				}
			}
			return triMesh;
		}
	}
	return TriangleMesh();
}

QuadMesh FileLoader::quadMeshLoader(const std::filesystem::path &path)
{
	if (path.extension() == ".obj")
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.string(), 0);
		for (unsigned int k = 0; k < scene->mNumMeshes; k++)
		{
			aiMesh* mesh = scene->mMeshes[k];
			QuadMesh quadMesh;
			quadMesh.indices.resize(mesh->mNumFaces * 4);
			quadMesh.faces.resize(mesh->mNumFaces);
			quadMesh.vertices.resize(mesh->mNumVertices);
			quadMesh.normals.resize(mesh->mNumVertices);
			quadMesh.uvcoords.resize(mesh->mNumVertices);
			quadMesh.colors.resize(mesh->mNumVertices);
			#pragma omp parallel for
			for (int i = 0; i < static_cast<int>(mesh->mNumFaces); i++)
			{
				quadMesh.faces.at(i) = glm::ivec4(mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2], mesh->mFaces[i].mIndices[3]);
				quadMesh.indices.at(i * 4) = mesh->mFaces[i].mIndices[0];
				quadMesh.indices.at(i * 4 + 1) = mesh->mFaces[i].mIndices[1];
				quadMesh.indices.at(i * 4 + 2) = mesh->mFaces[i].mIndices[2];
				quadMesh.indices.at(i * 4 + 3) = mesh->mFaces[i].mIndices[3];

			}
			#pragma omp parallel for
			for (int i = 0; i < static_cast<int>(mesh->mNumVertices); i++)
			{
				quadMesh.vertices.at(i) = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
				quadMesh.normals.at(i) = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 1.0f);
				if (mesh->HasTextureCoords(0))
				{
					quadMesh.uvcoords.at(i) = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				}
				if (mesh->HasVertexColors(0))
				{
					quadMesh.colors.at(i) = glm::vec4(mesh->mColors[i]->r, mesh->mColors[i]->g, mesh->mColors[i]->b, mesh->mColors[i]->a);
				}
			}
			return quadMesh;
		}
	}
	return QuadMesh();
}

Material FileLoader::loadImage(const std::filesystem::path & path, int width, int height)
{
	Material mat;
	GLuint textureHandle;
	unsigned char* image = stbi_load(path.string().c_str(), &width, &height, NULL, STBI_rgb);
	glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);
	glTextureParameteri(textureHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureStorage2D(textureHandle, 1, GL_RGB8, width, height);
	glTextureSubImage2D(textureHandle, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
	mat.textureAddress = glGetTextureHandleARB(textureHandle);
	glMakeTextureHandleResidentARB(mat.textureAddress);
	mat.color = glm::vec3(1.0, 1.0, 1.0);
	return mat;
}

FileLoader::~FileLoader()
{
}

