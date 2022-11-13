#pragma once
#include <vector>
#include "Material.h"
#include "VBO.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

    static float* toVBO(std::vector<Vertex> vertices);
};

class Mesh
{

public:
    // mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Material* material;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material);
    void Draw(Camera* camera);

    glm::mat4 model;

private:
    VBO* vbo;
    unsigned int vao, ebo;

    void setupMesh();

    unsigned int* ToEBO();

};

