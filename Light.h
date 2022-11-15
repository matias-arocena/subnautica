#pragma once
#include "GameObject.h"
#include <glm/glm.hpp>
#include "VBO.h"


class Light
{
	glm::vec3 color;
	glm::vec3 position;
public:
	Light(glm::vec3 color, glm::vec3 position);

	glm::vec3 getColor() const;
	glm::vec3 getPosition() const;
};

