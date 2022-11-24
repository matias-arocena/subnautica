#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"


class Water : public GameObject
{
private:
	glm::vec3 position;
	Mesh* mesh;
	Texture* refrTexture;
	Texture* reflTexture;
	Texture* reflectionDepthTexture;
	Texture* refractionDepthTexture;
	Texture* dudv_texture;
	unsigned int reflFrameBuffer;
	unsigned int refrFrameBuffer;
	float moveFactor;

public:
	Water();
	void update(double deltaTime) override;
	void renderLOD(LOD levelOfDetail) override;
	void renderOclussion() override;

	void unbindFrameFuffer();
	Material* initializeMaterial();
	void render_withShader(std::shared_ptr<Shader> shader);
	std::vector<std::shared_ptr<btRigidBody>> getCollisionObject();

};

