#include "Model.h"
#include <assimp/Importer.hpp>
#include "Utils.h"



#include "GameInstance.h"
#include "Definitions.h"
#include "GameInstance.h"

int& Model::getBoneCount()
{
	return m_BoneCounter;
}

std::vector<std::shared_ptr<Mesh>> Model::getMeshes() const
{
	return meshesLOD0;
}

void Model::setAnimator(std::shared_ptr<Animator> animator)
{
	this->animator = animator;
	for (auto &mesh : meshesLOD0)
	{
		mesh->setAnimator(animator);
	}
	for (auto& mesh : meshesLOD1)
	{
		mesh->setAnimator(animator);
	}
	for (auto& mesh : meshesLOD2)
	{
		mesh->setAnimator(animator);
	}
}

void Model::setGlobalAnimation(std::shared_ptr<GlobalAnimation> globalAnimation)
{
	for (auto& mesh : meshesLOD0)
	{
		mesh->setGlobalAnimation(globalAnimation);
	}
	for (auto& mesh : meshesLOD1)
	{
		mesh->setGlobalAnimation(globalAnimation);
	}
	for (auto& mesh : meshesLOD2)
	{
		mesh->setGlobalAnimation(globalAnimation);
	}

}

Model::Model(const std::string& path, const std::string& extension, glm::vec3 position) : GameObject()
{
	this->position = position;

	std::shared_ptr<Camera> camera = GameInstance::getInstance().getCamera();
	frustumLOD0 = camera->getFrustum(LOD::LOD0);
	frustumLOD1 = camera->getFrustum(LOD::LOD1);
	frustumLOD2 = camera->getFrustum(LOD::LOD2);

	loadModel(path + LOD_SUFFIX + "0." + extension, LOD::LOD0);
	loadModel(path + LOD_SUFFIX + "1." + extension, LOD::LOD1);
	loadModel(path + LOD_SUFFIX + "2." + extension, LOD::LOD2);

	this->animator = nullptr;

	for (auto& mesh : meshesLOD0)
	{
		mesh->move(position);
	}
	for (auto& mesh : meshesLOD1)
	{
		mesh->move(position);
	}
	for (auto& mesh : meshesLOD2)
	{
		mesh->move(position);
	}
}

Model::Model(const std::string& path, const std::string& extension, const std::string& animationPath, const std::string& animationExtension, glm::vec3 position) : GameObject()
{
	this->position = position;
	this->hasAnimations = true;
	std::shared_ptr<Camera> camera = GameInstance::getInstance().getCamera();
	frustumLOD0 = camera->getFrustum(LOD::LOD0);
	frustumLOD1 = camera->getFrustum(LOD::LOD1);
	frustumLOD2 = camera->getFrustum(LOD::LOD2);

	this->m_BoneInfoMap = std::make_shared<std::map<std::string, BoneInfo>>();
	// TODO: ARREGLAR
	loadModel(path + LOD_SUFFIX + "0." + extension, LOD::LOD0);
	loadModel(path + LOD_SUFFIX + "1." + extension, LOD::LOD1);
	loadModel(path + LOD_SUFFIX + "2." + extension, LOD::LOD2);

	loadAnimations(path + LOD_SUFFIX + "0." + extension);
	auto& animation = this->animations[0];
	this->setAnimator(std::make_shared<Animator>(animation));

	isMovable = true;
	for (auto& mesh : meshesLOD0)
	{
		mesh->move(position);
	}
	for (auto& mesh : meshesLOD1)
	{
		mesh->move(position);
	}
	for (auto& mesh : meshesLOD2)
	{
		mesh->move(position);
	}
}

void Model::render()
{
	for (std::shared_ptr<Mesh> mesh : meshesLOD0)
	{
		if (mesh->isOnFrustum(frustumLOD0))
			mesh->render();
	}
	
	for (std::shared_ptr<Mesh> mesh : meshesLOD1)
	{
		if (mesh->isOnFrustum(frustumLOD1) && !mesh->isOnFrustum(frustumLOD0))
			mesh->render();
	}

	for (std::shared_ptr<Mesh> mesh : meshesLOD2)
	{
		if (mesh->isOnFrustum(frustumLOD2) && !mesh->isOnFrustum(frustumLOD1))
			mesh->render();
	}

	for (ParticleGenerator& particleGenerator : particleGenerators)
	{
		particleGenerator.render();
	}

}

void Model::renderAABB()
{
	for (std::shared_ptr<Mesh> mesh : meshesLOD0)
	{
		mesh->renderAABB();
	}
}

void Model::renderWireframe()
{
	for (std::shared_ptr<Mesh> mesh : meshesLOD0)
	{
		if (mesh->isOnFrustum(frustumLOD0))
			mesh->renderWireframe();
	}

	for (std::shared_ptr<Mesh> mesh : meshesLOD1)
	{
		if (mesh->isOnFrustum(frustumLOD1) && !mesh->isOnFrustum(frustumLOD0))
			mesh->renderWireframe();
	}

	for (std::shared_ptr<Mesh> mesh : meshesLOD2)
	{
		if (mesh->isOnFrustum(frustumLOD2) && !mesh->isOnFrustum(frustumLOD0) && !mesh->isOnFrustum(frustumLOD1))
			mesh->renderWireframe();
	}

}

void Model::update(double DeltaTime)
{
	if (this->animator != nullptr)
		this->animator->updateAnimation(static_cast<float>(DeltaTime));
	
	for (std::shared_ptr<Mesh> mesh : meshesLOD0)
	{
		mesh->update(static_cast<float>(DeltaTime));
	}
	for (std::shared_ptr<Mesh> mesh : meshesLOD1)
	{
		mesh->update(static_cast<float>(DeltaTime));
	}
	for (std::shared_ptr<Mesh> mesh : meshesLOD2)
	{
		mesh->update(static_cast<float>(DeltaTime));
	}
	for (ParticleGenerator& particleGenerator : particleGenerators)
	{
		particleGenerator.update(DeltaTime);
	}
}

void Model::readMissingBones(const aiAnimation* assimpAnim, std::shared_ptr<Animation> animation)
{
	int size = assimpAnim->mNumChannels;
	
	int& boneCount = this->getBoneCount(); //getting the m_BoneCounter from Model class

	//reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++)
	{
		auto channel = assimpAnim->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (this->m_BoneInfoMap->find(boneName) == this->m_BoneInfoMap->end())
		{
			this->m_BoneInfoMap->at(boneName).id = boneCount;
			boneCount++;
		}
		animation->addBone(std::make_shared<Bone>(channel->mNodeName.data, this->m_BoneInfoMap->at(channel->mNodeName.data).id, channel));

		animation->setBoneInfoMap(this->m_BoneInfoMap);
	}
}

void Model::loadModel(const std::string& path, LOD lod)
{
	Assimp::Importer import;
	import.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);

	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | 
		aiProcess_FlipUVs | 
		aiProcess_GenSmoothNormals | 
		aiProcess_JoinIdenticalVertices | 
		aiProcess_CalcTangentSpace |
		aiProcess_GenBoundingBoxes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)

	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	// Initial trnasformation
	glm::mat4 transformMat = glm::identity<glm::mat4>();

	processNode(scene->mRootNode, scene, transformMat, lod, MeshType::NORMAL);
}

void Model::loadAnimations(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);

	for (size_t animation_c = 0; animation_c < scene->mNumAnimations; animation_c++)
	{
		auto animation_node = scene->mAnimations[animation_c];
		auto animation = std::make_shared<Animation>(scene->mRootNode, animation_node);
		readMissingBones(animation_node, animation);
		this->animations.push_back(animation);
	}
}

void Model::processNode(aiNode* node, const aiScene* scene, const glm::mat4& transformMat, LOD lod, MeshType meshType)
{
	// Combined transformations applied
	const glm::mat4 node_transformMat = transformMat * convertMatrix(node->mTransformation);


	if (std::string(node->mName.C_Str())._Starts_with(COLLISION_TAG))
	{
		meshType = MeshType::COLLISION;
	}
	//else if (std::string(node->mName.C_Str())._Starts_with(PARTICLE_TAG))
	//{
	//	meshType = MeshType::PARTICLE;
	//	glm::vec3 scale, translation, skew;
	//	glm::vec4 perspective;
	//	glm::quat rotation;
	//	glm::decompose(node_transformMat, scale, rotation, translation, skew, perspective);
	//	ParticleGenerator particleGenerator = ParticleGenerator(-translation);
	//	particleGenerators.push_back(particleGenerator);
	//	return;
	//}
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		//std::shared_ptr<Mesh> m = processMesh(mesh, scene, node_transformMat);

		switch (lod)
		{
		case LOD::LOD0:
			meshesLOD0.emplace_back(processMesh(mesh, scene, node_transformMat, meshType));
			break;
		case LOD::LOD1:
			meshesLOD1.emplace_back(processMesh(mesh, scene, node_transformMat, meshType));
			break;
		case LOD::LOD2:
			meshesLOD2.emplace_back(processMesh(mesh, scene, node_transformMat, meshType));
			break;
		default:
			break;
		}
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, node_transformMat, lod, meshType);
	}
}

void setVertexBoneData(Vertex& vertex, int boneID, float weight)
/*
	Set a bone that will influence a vertex
*/
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
	{
		if (vertex.m_BoneIDs[i] < 0)
		{
			vertex.m_Weights[i] = weight;
			vertex.m_BoneIDs[i] = boneID;
			break;
		}
	}
}

void Model::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh)
/*
	Attach the bones to the vertices, using the assimp mesh data
*/
{
	for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (m_BoneInfoMap->find(boneName) == m_BoneInfoMap->end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = m_BoneCounter;
			newBoneInfo.offset = convertMatrix(mesh->mBones[boneIndex]->mOffsetMatrix);
			m_BoneInfoMap->insert({ boneName, newBoneInfo });
			boneID = m_BoneCounter;
			m_BoneCounter++;
		}
		else
		{
			boneID = m_BoneInfoMap->at(boneName).id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		// Attach the bones to the vertices
		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			setVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}

std::shared_ptr<Mesh> Model::processMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transformMat, MeshType type)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<std::shared_ptr<Texture>> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		
		vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

		vertices.push_back(vertex);
	}

	for (int i = 0; i < static_cast<int>(mesh->mNumFaces); i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (hasAnimations)
		extractBoneWeightForVertices(vertices, mesh);

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN


	glm::vec3 diffuseColor, specularColor;
	diffuseColor = specularColor = glm::vec3(0);
	float specularStrenght, specularExponent;
	specularStrenght = specularExponent = 0;

	TextureLoader textureLoader = GameInstance::getInstance().getTextureLoader();
	
	textures.push_back(GameInstance::getInstance().shadowMapBuffer->shadowDepthTexture);

	// 2. specular maps
	std::vector<std::shared_ptr<Texture>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TEXTURE_SPECULAR);
	if (specularMaps.size() == 0)
	{
		aiColor3D aiSpecCol;
		// material->Get(AI_MATKEY_COLOR_SPECULAR, aiSpecCol); No anda
		material->Get(AI_MATKEY_SHININESS, specularExponent);
		// material->Get(AI_MATKEY_SHININESS_STRENGTH, specularStrenght);
		specularStrenght = 1.0;
		specularColor = glm::vec3(1.);
	}
	else
	{
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	// 1. diffuse maps
	std::vector<std::shared_ptr<Texture>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TEXTURE_DIFFUSE);
	if (diffuseMaps.size() == 0)
	{	
		aiColor3D aiDiff;
		material->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiff);
		diffuseColor = toVec3(aiDiff);
	}
	else
	{
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	}

	// 3. normal maps
	std::vector<std::shared_ptr<Texture>> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, TEXTURE_NORMAL);
	if (normalMaps.size() == 0)
	{
		normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TEXTURE_NORMAL);
	}

	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps
	std::vector<std::shared_ptr<Texture>> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	textures.push_back(textureLoader.getTexture("assets/caustics.jpg", "occlusion_map"));
	textures.push_back(textureLoader.getTexture("assets/causticsDUDV.png", "dudv_map"));
	textures.push_back(textureLoader.getTexture("assets/causticFactor.jpg", "caustics_factor"));

	Material *m = new Material(textures, GameInstance::getInstance().getShader(NORMAL_SHADER), diffuseColor, specularColor, specularStrenght, specularExponent);
	
	//glm::vec4 AABBModel =  mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z, 1.0f };
	glm::vec3 AABBmin = { mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z };
	glm::vec3 AABBmax = { mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z };

	return std::make_shared<Mesh>(vertices, indices, m, transformMat, AABBmin, AABBmax, type);
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	TextureLoader textureLoader = GameInstance::getInstance().getTextureLoader();
	std::vector<std::shared_ptr<Texture>> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string filename;
		filename = this->directory + '/' + str.C_Str();

		auto texture = textureLoader.getTexture(filename, typeName);
		
		textures.push_back(texture);
		textures_loaded.push_back(texture);
	}
	return textures;

}

void Model::clipModel(const glm::vec4& plane)
{
	for (std::shared_ptr<Mesh> m : meshesLOD0)
	{
		m->setClipPlane(plane);
	}
	for (std::shared_ptr<Mesh> m : meshesLOD1)
	{
		m->setClipPlane(plane);
	}
	for (std::shared_ptr<Mesh> m : meshesLOD2)
	{
		m->setClipPlane(plane);
	}
}

void Model::render_withShader(std::shared_ptr<Shader> shader)
{
	for (std::shared_ptr<Mesh> mesh : meshesLOD0)
	{
		mesh->render_withShader(shader);
	}
}

void Model::renderOclussion()
{
	printf("No esta hecho"); // Ya se que es una cagada esto 
}


