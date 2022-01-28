#pragma once

#include <string>
#include "definitions.h"
#include "object.h"
#include "tinyOBJ\tiny_obj_loader.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <assimp\mesh.h>
#include <stbImage\stb_image.h>
#include "shaderProgram.h"

enum class FileType {obj};

 class FileLoader 
{
public:
	FileLoader();
	static TriangleMesh triangleMeshLoader(const std::filesystem::path &path);
	static QuadMesh quadMeshLoader(const std::filesystem::path &path);
	static Material loadImage(const std::filesystem::path &path, int width, int height);
	~FileLoader();
private:
};

