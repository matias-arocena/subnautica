#include "Camera.h"
#include "btBulletDynamicsCommon.h"
#include "ConfigManager.h"
#include "Mesh.h"

Camera::Camera(
    glm::vec3 position,
    glm::vec3 up,
    float yaw, float pitch
) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    
    frustumLOD0 = std::make_shared<Frustum>();
    frustumLOD1 = std::make_shared<Frustum>();
    frustumLOD2 = std::make_shared<Frustum>();

    createViewFrustum();

    updateCameraVectors();
    ProjectionMatrix = glm::perspective(glm::radians(Zoom), width / height, near, far);

}

Camera::Camera(
    float posX, float posY, float posZ,
    float upX, float upY, float upZ,
    float yaw, float pitch
) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;

    frustumLOD0 = std::make_shared<Frustum>();
    frustumLOD1 = std::make_shared<Frustum>();
    frustumLOD2 = std::make_shared<Frustum>();

    createViewFrustum();
    updateCameraVectors();
    ProjectionMatrix = glm::perspective(glm::radians(Zoom), width / height , near, far);
}


void Camera::createViewFrustum()
{
    const float halfVSide = far * std::tanf(glm::radians(Zoom) * .5f);
    const float halfHSide = halfVSide * (width / height);
    const glm::vec3 frontMultFar = far * Front;

    Plane nearFace   = { Position + /*near * */ 0.f * Front, Front};
    Plane rightFace  = { Position,
                       glm::cross(Up,frontMultFar + Right * halfHSide) };
    Plane leftFace   = { Position,
                       glm::cross(frontMultFar - Right * halfHSide, Up) };
    Plane topFace    = { Position,
                       glm::cross(Right, frontMultFar - Up * halfVSide) };
    Plane bottomFace = { Position,
                       glm::cross(frontMultFar + Up * halfVSide, Right) };

    frustumLOD0->nearFace = nearFace;
    frustumLOD0->farFace = { Position + frontMultFar, -Front };
    frustumLOD0->rightFace = rightFace;
    frustumLOD0->leftFace = leftFace;
    frustumLOD0->topFace = topFace;
    frustumLOD0->bottomFace = bottomFace;

    frustumLOD1->nearFace = nearFace;
    frustumLOD1->farFace = { Position + frontMultFar, -Front };
    frustumLOD1->rightFace = rightFace;
    frustumLOD1->leftFace = leftFace;
    frustumLOD1->topFace = topFace;
    frustumLOD1->bottomFace = bottomFace;

    frustumLOD2->nearFace = nearFace;
    frustumLOD2->farFace = { Position + frontMultFar, -Front };
    frustumLOD2->rightFace = rightFace;
    frustumLOD2->leftFace = leftFace;
    frustumLOD2->topFace = topFace;
    frustumLOD2->bottomFace = bottomFace;
}

void Camera::updateViewMatrix()
{
    ViewMatrix = glm::lookAt(Position, Position + Front, Up);
}

void Camera::toggleFrustumUpdate()
{
    shouldFrustumUpdate = !shouldFrustumUpdate;
}

std::shared_ptr<Frustum> Camera::getFrustum(LOD lod)
{
    if (lod == LOD::LOD0)
    {
        return frustumLOD0;
    }
    else if (lod == LOD::LOD1)
    {
        return frustumLOD1;
    }
    else
    {
        return frustumLOD2;
    }
}

void Camera::changeSize(glm::ivec2 size)
{
    this->width  = size.x;
    this->height = size.y;
    ProjectionMatrix = glm::perspective(glm::radians(Zoom), width / height, near, far);

    if (shouldFrustumUpdate)
        createViewFrustum();

}


glm::mat4 Camera::GetViewMatrix() const
{
    return ViewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return ProjectionMatrix;
}

glm::vec4 Camera::GetPosition() const
{
    return glm::vec4(Position, 1.f);
}

void Camera::SetPosition(glm::vec3 position) 
{
    this->Position = position;
}

void Camera::InvertPitch()
{
    this->Pitch = -Pitch;
    updateCameraVectors();
}


void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw = glm::mod(Yaw + xoffset, 360.0f);
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 90.0f)
        Zoom = 90.0f;
	float aspect = width / height;
	
    ProjectionMatrix = glm::perspective(glm::radians(Zoom), aspect, 0.1f, 100.0f);

    if (shouldFrustumUpdate)
        createViewFrustum();

}


void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));

    glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
    
    if(shouldFrustumUpdate)
        createViewFrustum();
}