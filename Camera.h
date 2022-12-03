#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include "VBO.h"
#include "ConfigManager.h"

class Player;

struct Plane
{
    glm::vec3 normal = { 0.f, 1.f, 0.f };
    float distance = 0.f;
    glm::vec3 m_point = { 0.f, 0.f, 0.f };

    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1)),
        m_point(p1)
    {}

    Plane() = default;

    inline glm::vec4 equation() 
    {
        return { normal, distance };
    }

    inline float getSignedDistanceToPlan(const glm::vec3& point) const
    {
        return glm::dot(normal, point - this->m_point);
//        return glm::dot(normal, point) - distance;
    }

    static glm::vec3 intersectPlanes(const Plane& a, const Plane& b, const Plane& c);

};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;
    Plane rightFace;
    Plane leftFace;
    Plane farFace;
    Plane nearFace;

    glm::vec3 points[8];
    bool isBoxVisible(const glm::vec3& minp, const glm::vec3& maxp);
    
};

inline bool Frustum::isBoxVisible(const glm::vec3& minp, const glm::vec3& maxp) 
{
    return true;
    // check box outside/inside of frustum
    if ((glm::dot(topFace.equation(), glm::vec4(minp.x, minp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(maxp.x, minp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(minp.x, maxp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(maxp.x, maxp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(minp.x, minp.y, maxp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(maxp.x, minp.y, maxp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(minp.x, maxp.y, maxp.z, 1.0f))    > 0.4) &&
        (glm::dot(topFace.equation(), glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f))    > 0.4))
    {
        return false;
    }
    if ((glm::dot(bottomFace.equation(), glm::vec4(minp.x, minp.y, minp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(maxp.x, minp.y, minp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(minp.x, maxp.y, minp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(maxp.x, maxp.y, minp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(minp.x, minp.y, maxp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(maxp.x, minp.y, maxp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(minp.x, maxp.y, maxp.z, 1.0f)) > 0.4) &&
        (glm::dot(bottomFace.equation(), glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f)) > 0.4))
    {
        return false;
    }
    if ((glm::dot(rightFace.equation(), glm::vec4(minp.x, minp.y, minp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(maxp.x, minp.y, minp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(minp.x, maxp.y, minp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(maxp.x, maxp.y, minp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(minp.x, minp.y, maxp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(maxp.x, minp.y, maxp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(minp.x, maxp.y, maxp.z, 1.0f))  > 0.4) &&
        (glm::dot(rightFace.equation(), glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f))  > 0.4))
    {
        return false;
    }
    if ((glm::dot(leftFace.equation(), glm::vec4(minp.x, minp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(maxp.x, minp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(minp.x, maxp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(maxp.x, maxp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(minp.x, minp.y, maxp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(maxp.x, minp.y, maxp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(minp.x, maxp.y, maxp.z, 1.0f))   > 0.4) &&
        (glm::dot(leftFace.equation(), glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f))   > 0.4))
    {
        return false;
    }
    if ((glm::dot(farFace.equation(), glm::vec4(minp.x, minp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(maxp.x, minp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(minp.x, maxp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(maxp.x, maxp.y, minp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(minp.x, minp.y, maxp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(maxp.x, minp.y, maxp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(minp.x, maxp.y, maxp.z, 1.0f))    > 0.4) &&
        (glm::dot(farFace.equation(), glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f))    > 0.4))
    {
        return false;
    }
    if ((glm::dot(nearFace.equation(), glm::vec4(minp.x, minp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(maxp.x, minp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(minp.x, maxp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(maxp.x, maxp.y, minp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(minp.x, minp.y, maxp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(maxp.x, minp.y, maxp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(minp.x, maxp.y, maxp.z, 1.0f))   > 0.4) &&
        (glm::dot(nearFace.equation(), glm::vec4(maxp.x, maxp.y, maxp.z, 1.0f))   > 0.4))
    {
        return false;
    }


    // check frustum outside/inside box
    int out;
    out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x > maxp.x) ? 1 : 0); if (out == 8) return false;
    out = 0; for (int i = 0; i < 8; i++) out += ((points[i].x < minp.x) ? 1 : 0); if (out == 8) return false;
    out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y > maxp.y) ? 1 : 0); if (out == 8) return false;
    out = 0; for (int i = 0; i < 8; i++) out += ((points[i].y < minp.y) ? 1 : 0); if (out == 8) return false;
    out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z > maxp.z) ? 1 : 0); if (out == 8) return false;
    out = 0; for (int i = 0; i < 8; i++) out += ((points[i].z < minp.z) ? 1 : 0); if (out == 8) return false;

    return true;
}

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera
{
    float height = 480;
    float width = 640;
    float near = 0.1f;
    float far = 200.f;
    std::shared_ptr<Frustum> frustumLOD0;
    std::shared_ptr<Frustum> frustumLOD1;
    std::shared_ptr<Frustum> frustumLOD2;
    Mode mode = Mode::FLY_MODE;
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;

    std::shared_ptr<Player> player;

    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    // constructor with vectors
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW, float pitch = PITCH
    );
    // constructor with scalar values
    Camera(
        float posX, float posY, float posZ,
        float upX, float upY, float upZ,
        float yaw, float pitch
    );
    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() const;
    void SetPosition(const glm::vec3& position);
    void InvertPitch();
    glm::mat4 GetProjectionMatrix() const;
    glm::vec4 GetPosition() const; 
    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);
    void updateViewMatrix();

	void changeSize(const glm::ivec2& size);

    std::shared_ptr<Frustum> getFrustum(enum class LOD lod);
    void createViewFrustum();

    void renderFrustum();
    void toggleFrustumUpdate();
private:
    std::shared_ptr<class Shader> frustumShader;
    glm::mat4 frustumModel = glm::identity<glm::mat4>();
    unsigned int vao, ebo, indicesSize;
    std::unique_ptr<VBO> vbo;

    bool shouldFrustumUpdate = false;
    void updateViewFrustum();

    void updateCameraVectors();
};
