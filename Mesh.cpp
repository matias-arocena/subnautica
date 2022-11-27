#include "Mesh.h"
#include "GameInstance.h"
#include <GLFW/glfw3.h>
#include "Definitions.h"

int Vertex::numElementsInVBO = 14;

Mesh::Mesh(std::vector<Vertex> verticesLOD0, std::vector<unsigned int> indicesLOD0,
	std::vector<Vertex> verticesLOD1, std::vector<unsigned int> indicesLOD1,
	std::vector<Vertex> verticesLOD2, std::vector<unsigned int> indicesLOD2,
	Material* material, glm::mat4 modelMat, glm::vec3 min, glm::vec3 max)
{
	this->verticesLOD0 = verticesLOD0;
	this->indicesLOD0 = indicesLOD0;
	this->verticesLOD1 = verticesLOD1;
	this->indicesLOD1 = indicesLOD1;
	this->verticesLOD2 = verticesLOD2;
	this->indicesLOD2 = indicesLOD2;

	this->material = material;
	clipPlane = glm::vec4{ 0.f, 0.f, 0.f, 0.f };
	this->model = modelMat;
	center = { (max + min) * 0.5f };
	extents = { max.x - center.x, max.y - center.y, max.z - center.z };

	std::shared_ptr<Camera> camera = GameInstance::getInstance().getCamera();
	frustumLOD0 = camera->getFrustum(LOD::LOD0);
	frustumLOD1 = camera->getFrustum(LOD::LOD1);
	frustumLOD2 = camera->getFrustum(LOD::LOD2);

	vboLOD0 = new VBO();
	vboLOD1 = new VBO();
	vboLOD2 = new VBO();

	setupMesh(verticesLOD0, indicesLOD0, *vboLOD0, vaoLOD0, eboLOD0);
	setupMesh(verticesLOD1, indicesLOD1, *vboLOD1, vaoLOD1, eboLOD1);
	setupMesh(verticesLOD2, indicesLOD2, *vboLOD2, vaoLOD2, eboLOD2);

	debugShader = GameInstance::getInstance().getShader(FRUSTUM_SHADER);
	Vertex nbl({ center.x - extents.x, center.y - extents.y, center.z + extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex nbr({ center.x + extents.x, center.y - extents.y, center.z + extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex ntl({ center.x - extents.x, center.y + extents.y, center.z + extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex ntr({ center.x + extents.x, center.y + extents.y, center.z + extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex fbl({ center.x - extents.x, center.y - extents.y, center.z - extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex fbr({ center.x + extents.x, center.y - extents.y, center.z - extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex ftl({ center.x - extents.x, center.y + extents.y, center.z - extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });
	Vertex ftr({ center.x + extents.x, center.y + extents.y, center.z - extents.z }, { 0.f, 1.f, 0.f }, { 0.f,1.f });

	std::vector<Vertex> vertices = {
		nbl/*0*/, nbr/*1*/, ntl/*2*/, ntr/*3*/,
		fbl/*4*/, fbr/*5*/, ftl/*6*/, ftr/*7*/
	};
	std::vector<unsigned int> indices = {
		//near plane
		0, 1, 1, 3, 3, 2, 2, 0,
		//far plane
		4, 5, 5, 7, 7, 6, 6, 4,
		//bottom plane
		0, 4, 4, 5, 5, 1, 1, 0,
		//top plane
		2, 3, 3, 7, 7, 6, 6, 2
	};

	debugIndicesSize = indices.size();

	glGenVertexArrays(1, &debugVao);
	glGenBuffers(1, &debugEbo);

	glBindVertexArray(debugVao);

	debugVBO = std::make_unique<VBO>();
	debugVBO->load(Vertex::toVBO(vertices), vertices.size() * Vertex::numElementsInVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), VBO::toEBO(indices), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);
}

void Mesh::render()
{
	LOD lod = getLOD();
	if (lod == LOD::NotInFrustum)
		return;

	material->use();

	std::shared_ptr<Shader> shader = material->getShader();
	shader->setFloat("time", glfwGetTime());
	//esto es por ahora 
	shader->setFloat("clippingPlane", clipPlane.x, clipPlane.y, clipPlane.z, clipPlane.w);
	shader->setMat4("model", model);

	bindToLOD(lod);

	if (GameInstance::getInstance().isDebugMode())
	{
		debugShader->setFloat("color", 0.f, 1.f, 0.f);
		debugShader->setMat4("model", model);

		glBindVertexArray(debugVao);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_LINES, debugIndicesSize, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

void Mesh::setupMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, VBO& vbo, unsigned int& vao, unsigned int& ebo)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &ebo);

	// 1. bind Vertex Array Object
	glBindVertexArray(vao);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	vbo.load(Vertex::toVBO(vertices), vertices.size() * Vertex::numElementsInVBO); // Vertices and qty
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), VBO::toEBO(indices), GL_STATIC_DRAW);
	
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, Vertex::numElementsInVBO * sizeof(float), (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

}

void Mesh::bindToLOD(LOD lod)
{
	if (lod == LOD::LOD0)
	{
		glBindVertexArray(vaoLOD0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, indicesLOD0.size(), GL_UNSIGNED_INT, 0); // cantidad de indices
		glBindVertexArray(0);
	}
	else if (lod == LOD::LOD1)
	{
		glBindVertexArray(vaoLOD1);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, indicesLOD1.size(), GL_UNSIGNED_INT, 0); // cantidad de indices
		glBindVertexArray(0);
	}
	else 
	{
		glBindVertexArray(vaoLOD2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, indicesLOD2.size(), GL_UNSIGNED_INT, 0); // cantidad de indices
		glBindVertexArray(0);
	}
	
}



LOD Mesh::getLOD()
{
	//Get global scale thanks to our transform
	const glm::vec3 globalCenter{ model * glm::vec4(center, 1.f) };

	// Scaled orientation
	const glm::vec3 right = model[0] * extents.x;
	const glm::vec3 up = model[1] * extents.y;
	const glm::vec3 forward = -model[2] * extents.z;

	const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
		std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
		std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

	const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

	const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
		std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));
	const glm::vec3 extents = { newIi, newIj, newIk };

	if (isOnFrustum(globalCenter, extents, frustumLOD0))
	{
		return LOD::LOD0;
	}
	else if (isOnFrustum(globalCenter, extents, frustumLOD1))
	{
		return LOD::LOD1;
	}
	else if (isOnFrustum(globalCenter, extents, frustumLOD2))
	{
		return LOD::LOD2;
	}
	else
	{
		return LOD::NotInFrustum;
	}
}

bool Mesh::isOnFrustum(glm::vec3 center, glm::vec3 extents, std::shared_ptr<Frustum> frustum)
{
	bool left = isOnOrForwardPlane(center, extents, frustum->leftFace);
	bool right = isOnOrForwardPlane(center, extents, frustum->rightFace);
	bool top = isOnOrForwardPlane(center, extents, frustum->topFace);
	bool bottom = isOnOrForwardPlane(center, extents, frustum->bottomFace);
	bool near = isOnOrForwardPlane(center, extents, frustum->nearFace);
	bool far = isOnOrForwardPlane(center, extents, frustum->farFace);

	bool res = left && right && top && bottom && near && far;


	return isOnOrForwardPlane(center, extents, frustum->leftFace) &&
		isOnOrForwardPlane(center, extents, frustum->rightFace) &&
		isOnOrForwardPlane(center, extents, frustum->topFace) &&
		isOnOrForwardPlane(center, extents, frustum->bottomFace) &&
		isOnOrForwardPlane(center, extents, frustum->nearFace) &&
		isOnOrForwardPlane(center, extents, frustum->farFace);
}

float* Vertex::toVBO(std::vector<Vertex> verticesLOD0)
{
	float* vboLOD1 = new float[verticesLOD0.size() * Vertex::numElementsInVBO];
	
	for (size_t i = 0; i < verticesLOD0.size(); ++i)
	{
		vboLOD1[i * Vertex::numElementsInVBO     ] = verticesLOD0[i].Position.x;
		vboLOD1[i * Vertex::numElementsInVBO +  1] = verticesLOD0[i].Position.y;
		vboLOD1[i * Vertex::numElementsInVBO +  2] = verticesLOD0[i].Position.z;
		vboLOD1[i * Vertex::numElementsInVBO +  3] = verticesLOD0[i].Normal.x;
		vboLOD1[i * Vertex::numElementsInVBO +  4] = verticesLOD0[i].Normal.y;
		vboLOD1[i * Vertex::numElementsInVBO +  5] = verticesLOD0[i].Normal.z;
		vboLOD1[i * Vertex::numElementsInVBO +  6] = verticesLOD0[i].TexCoords.x;
		vboLOD1[i * Vertex::numElementsInVBO +  7] = verticesLOD0[i].TexCoords.y;
		vboLOD1[i * Vertex::numElementsInVBO +  8] = verticesLOD0[i].Tangent.x;
		vboLOD1[i * Vertex::numElementsInVBO +  9] = verticesLOD0[i].Tangent.y;
		vboLOD1[i * Vertex::numElementsInVBO + 10] = verticesLOD0[i].Tangent.z;
		vboLOD1[i * Vertex::numElementsInVBO + 11] = verticesLOD0[i].Bitangent.x;
		vboLOD1[i * Vertex::numElementsInVBO + 12] = verticesLOD0[i].Bitangent.y;
		vboLOD1[i * Vertex::numElementsInVBO + 13] = verticesLOD0[i].Bitangent.z;

	}

	return vboLOD1;
}

void Mesh::setClipPlane(glm::vec4 plane)
{
	this->clipPlane = plane;
}

glm::vec4 Mesh::getClipPlane()
{
	return this->clipPlane;
}

void Mesh::render_withShader(std::shared_ptr<Shader> shader)
{
	LOD lod = getLOD();
	if (lod == LOD::NotInFrustum)
		return;

	shader->use();
	shader->setFloat("clippingPlane", clipPlane.x, clipPlane.y, clipPlane.z, clipPlane.w);

	shader->setMat4("model", model);

	bindToLOD(lod);

}

void Mesh::toggleDebugAABB()
{
	debugAABB = !debugAABB;
}


