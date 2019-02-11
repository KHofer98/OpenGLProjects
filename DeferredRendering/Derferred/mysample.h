#pragma once

#include "sample.h"

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "glsl.h"
#include "model.h"

using std::string;

class MySample : public cg2::Sample
{
public:

	MySample(unsigned int w, unsigned int h);

	void renderFrame();

	void renderCube();

	void init();

	void cleanup();

	void resize(const unsigned w, const unsigned h);

	void mouseButton(const int button, const int action, const int mods);

	void mousePosition(const double xpos, const double ypos);

	void keyInput(const int key, const int scancode, const int action, const int mods);

	void mouseScroll(const double xoffset, const double yoffset);

	void renderQuad();

private:
	std::shared_ptr<cg2::GlslProgram> prog;
	std::shared_ptr<cg2::GlslProgram> deferrerdshading;
	std::shared_ptr<cg2::GlslProgram> light;

	string FRAGMENT_SHADER_PFAD = R"(shader_temp.frag)";
	string VERTEX_SHADER_PFAD = R"(shader.vert)";
	string VERTEX_SHADER_DEF_PFAD = R"(deferred_shading.vert)";
	string FRAGMENT_SHADER_DEF_PFAD = R"(deferred_shading.frag)";
	string VERTEX_SHADER_LIGHT_PFAD = R"(light.vert)";
	string FRAGMENT_SHADER_LIGHT_PFAD = R"(light.frag)";
	int windowHeight;
	int windowWidth;

	float deltaTime;
	float lastFrame;

	unsigned mHandleToDepthTexture = 0;
	unsigned mHandleToFramebufferObject = 0;
	unsigned mFBOWidth = 1024;
	unsigned mFBOHeight = 1024;

	std::shared_ptr<const cg2::Model> mModel;
	glm::mat4 model;
	std::vector<glm::vec3> objectPositions;
	std::vector<glm::vec3> cubePositions;
	unsigned int texture1, texture2;
	unsigned int cubeVAO  = 0;
	unsigned int cubeVBO = 0;
	unsigned int VBO;
	unsigned int quadVAO = 0;
	unsigned int quadVBO;

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	float fov;
	float cameraSpeed;

	bool firstMouse;
	float yaw;
	float pitch;
	float lastX;
	float lastY;
	bool isPressedW;
	bool isPressedS;
	bool isPressedA;
	bool isPressedD;

	const unsigned int NR_LIGHTS = 32;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;

	unsigned int gBuffer;
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gColorSpec;
};