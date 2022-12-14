#include "Loader.h"
#include <iostream>
#include <memory>

#include "ConfigManager.h"
#include "GameInstance.h"
#include "Light.h"
#include "Player.h"
#include "OrbitAnimation.h"


void Loader::loadScene()
{
	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file("assets/config.xml");

	if (!result)
	{
		std::cout << "Failed to load scene xml" << std::endl;
		std::cout << "Error description: " << result.description() << "\n";
		return;
	}


	auto& config = ConfigManager::getInstance();
	//General Settings
	config.setWindowSize({ 
		doc.child("Scene").attribute("resX").as_int(),
		doc.child("Scene").attribute("resY").as_int()
	});

	config.setClearColor({
		doc.child("Scene").attribute("bgColorR").as_int(),
		doc.child("Scene").attribute("bgColorG").as_int(),
		doc.child("Scene").attribute("bgColorB").as_int(),
	});

	config.setOcclusionMapResolution({
		doc.child("Scene").attribute("occlusionResX").as_int(),
		doc.child("Scene").attribute("occlusionResY").as_int()
		});

	config.setWaterReflectionResolution({
		doc.child("Scene").attribute("waterResX").as_int(),
		doc.child("Scene").attribute("waterResY").as_int()
	});

	config.setFar(doc.child("Scene").attribute("far").as_float());
	config.setNear(doc.child("Scene").attribute("near").as_float());
	
	loadLight(doc.child("Scene"));
	loadPlayer(doc.child("Scene"));
	loadModels(doc.child("Scene"));

	config.setMode(doc.child("Scene").attribute("mode").as_string() == "player" ? PLAYER_MODE : FLY_MODE);

#ifdef _DEBUG
	std::cout << "Scene loaded" << std::endl;
#endif
}

void Loader::loadPlayer(pugi::xml_node node)
{
	GameInstance& gameInstance = GameInstance::getInstance();

	if (node.attribute("animationPath"))
	{
		std::string animationPath = node.attribute("animationPath").as_string();
		std::string animationExtension = node.attribute("animationExtension").as_string();

		gameInstance.setPlayer(std::make_shared<Player>(
			node.child("Player").attribute("modelPath").as_string(),
			node.child("Player").attribute("modelExtension").as_string(),
			animationPath, animationExtension));
	}
	else
	{
	gameInstance.setPlayer(std::make_shared<Player>(
		node.child("Player").attribute("modelPath").as_string(),
		node.child("Player").attribute("modelExtension").as_string()
	));
}

}

void Loader::loadModels(pugi::xml_node node)
{
	GameInstance& gameInstance = GameInstance::getInstance();

	for (pugi::xml_node obj : node.child("Models"))
	{
		std::string objType = obj.name();

		if (objType.compare("Model") == 0)
		{

			glm::vec3 position(0, 0, 0);
			position.x = obj.attribute("posx").as_float();
			position.y = obj.attribute("posy").as_float();
			position.z = obj.attribute("posz").as_float();

			std::string modelPath = obj.attribute("modelPath").as_string();
			std::string modelExtension = obj.attribute("modelExtension").as_string();
			
			std::shared_ptr<Model> model;
			
			if (obj.attribute("animationPath"))
			{
				std::string animationPath = obj.attribute("animationPath").as_string();
				std::string animationExtension = obj.attribute("animationExtension").as_string();
				model = std::make_shared<Model>(modelPath, modelExtension, animationPath, animationExtension, position);
				
			}
			else
			{
				model = std::make_shared<Model>(modelPath, modelExtension, position);
			}

			if (obj.attribute("animationType"))
			{
				std::string animationType = obj.attribute("animationType").as_string();
				if (animationType.compare("orbit") == 0)
				{
					glm::vec3 orbitCenter;
					float radius = obj.attribute("radius").as_float();
					float orbitSpeed = obj.attribute("orbitSpeed").as_float();
					bool reverse = obj.attribute("reverse").as_bool();
					
					// Read from xml
					orbitCenter = glm::vec3(
						obj.attribute("orbitCenterX").as_float(),
						obj.attribute("orbitCenterY").as_float(),
						obj.attribute("orbitCenterZ").as_float()
					);
						
					model->setGlobalAnimation(std::make_shared<OrbitAnimation>(orbitCenter, radius, orbitSpeed, reverse));
				}
				
			}
			gameInstance.addGameObject(model);
		}
	}

}

void Loader::loadLight(pugi::xml_node node)
{
	node = node.child("Light");
	std::string objType = node.name();

	if (objType.compare("Light") == 0)
	{

		glm::vec3 color;

		color.r = node.attribute("colorr").as_float();
		color.g = node.attribute("colorg").as_float();
		color.b = node.attribute("colorb").as_float();


		glm::vec3 direction;
		direction.x = node.attribute("x").as_float();
		direction.y = node.attribute("y").as_float();
		direction.z = node.attribute("z").as_float();

		std::shared_ptr<Light> light = std::make_shared<Light>(color, direction);

		GameInstance::getInstance().addLight(light);
	}
}
