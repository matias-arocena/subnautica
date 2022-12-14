#include "SkyBox.h"
#include "stb_image.h"
#include <iostream>
#include "Definitions.h"
#include "GameInstance.h"
#include <array>

SkyBox::SkyBox()
{
	GameInstance::getInstance().getShader(SKY_BOX_SHADER);
	this->shader = GameInstance::getInstance().getShader(SKY_BOX_SHADER);
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	std::array<std::string, 6> paths{ "right", "left", "top", "bottom", "back", "front" };

	for (int i = 0; i < paths.size(); i++)
	{
		std::string path = SKY_BOX_TEXT_PATH + paths[i] + ".jpg";
		int width, height, nrChannels = 0;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Failed to load SKY BOX texture " << path.c_str() << std::endl;
        }
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	this->shader->use();
	this->shader->setInt("cube_map", 0);

	glGenVertexArrays(1, &vaoLOD1);
	vboLOD1 = new VBO();
	glBindVertexArray(vaoLOD1);
	vboLOD1->load(VERTICES, 108); 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void SkyBox::renderOclussion()
{
	return;
}


void SkyBox::update(double DeltaTime)
{

}

void SkyBox::render()
{
	this->shader->use();

	glBindVertexArray(vaoLOD1);
	glEnableVertexAttribArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);

}


void SkyBox::render_withShader(std::shared_ptr<Shader> shader)
{

}
