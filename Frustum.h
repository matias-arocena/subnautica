#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "VBO.h"

struct FrustumVertices
{
    glm::vec3 farTopLeft;
    glm::vec3 farTopRight;
    glm::vec3 farBottomLeft;
    glm::vec3 farBottomRight;
    glm::vec3 nearTopLeft;
    glm::vec3 nearTopRight;
    glm::vec3 nearBottomLeft;
    glm::vec3 nearBottomRight;
};

struct FrustumPlanes
{
    glm::vec4 near;
    glm::vec4 far;
    glm::vec4 top;
    glm::vec4 bottom;
    glm::vec4 left;
    glm::vec4 right;
};

class Frustum
{
    glm::vec3 position;

    glm::vec3 frontMultNear;
    glm::vec3 frontMultFar;

    glm::vec3 farTopLeftSum;
    glm::vec3 farTopRightSum;
    glm::vec3 farBottomLeftSum;
    glm::vec3 farBottomRightSum;
    glm::vec3 nearTopLeftSum; 
    glm::vec3 nearTopRightSum;
    glm::vec3 nearBottomLeftSum;
    glm::vec3 nearBottomRightSum;

    glm::vec4 nearPlane, farPlane, bottomPlane, topPlane, leftPlane, rightPlane;

    glm::vec3 front, up;
    float near, far, fov;

    bool freeze = false;

    FrustumVertices calculateVertices() const;
   
    FrustumVertices toWorldCoordinates(const FrustumVertices& vertices);
    inline glm::vec3 toWorldCoordinates(const glm::vec3& vertex);
    inline glm::mat4 getModelMatrix();

    inline static glm::vec4 planeEquation(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);

    unsigned int vao, ebo, indicesSize;
    std::shared_ptr<VBO> vbo;
    std::shared_ptr<class Shader> frustumShader;

    void setupRender();
public:
    Frustum(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up, const glm::vec3& right, float fov, float near, float far, float aspect);
    void update(const glm::vec3& position, const glm::vec3& front, const glm::vec3& right, const glm::vec3& up, float aspect);
    void update(const glm::vec3& front, const glm::vec3& right, const glm::vec3& up, float aspect);
    void update(const glm::vec3& position);
    void update(float fov, float aspect, glm::vec3 right);
    void render(const glm::vec3& color = {0.f, 1.f, 0.f});
    bool isBoxInFrustum(const glm::vec3& minp, const glm::vec3& maxp) const;
    void toggleFreeze();
};

inline glm::vec3 Frustum::toWorldCoordinates(const glm::vec3& vertex)
{
    
    return glm::vec3(getModelMatrix() * glm::vec4(vertex, 1.0f));
}

inline glm::mat4 Frustum::getModelMatrix()
{
    return glm::inverse(glm::lookAt(position, position + front, up));
}

inline glm::vec4 Frustum::planeEquation(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
    const glm::vec3 v = p2 - p1;
    const glm::vec3 w = p3 - p1;
    
    const glm::vec3 n = glm::cross(v, w);

    return glm::vec4(n, -n.x * p1.x - n.y * p1.y - n.z * p1.z);
}