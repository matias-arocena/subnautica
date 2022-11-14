#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"

class Model
{

private:
	std::vector<Mesh> meshes;
	std::string directory;
    std::vector<Texture*> textures_loaded;

public:
    Model(std::string path);
    ~Model();
    void draw(Camera* camera);
    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    void update(double DeltaTime);
};
