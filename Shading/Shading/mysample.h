#pragma once

#include "sample.h"

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "glsl.h"

using std::string;

class MySample : public cg2::Sample
{
public:

	MySample(unsigned int w, unsigned int h);

	void renderFrame();

	void init();

	void cleanup();

	void resize(const unsigned w, const unsigned h);

	void mouseButton(const int button, const int action, const int mods);

	void mousePosition(const double xpos, const double ypos);

	void keyInput(const int key, const int scancode, const int action, const int mods);

	void mouseScroll(const double xoffset, const double yoffset);

private:
	int windowHeight;
	int windowWidth;
	std::shared_ptr<cg2::GlslProgram> prog;
	std::shared_ptr<cg2::GlslProgram> lamp;
	std::shared_ptr<cg2::GlslProgram> depth;
	std::shared_ptr<cg2::GlslProgram> renderdepth;
	string VERTEX_SHADER_PFAD = R"(shader.vert)";
	string FRAGMENT_SHADER_PFAD = R"(shader.frag)";
	string VERTEX_LAMP_SHADER_PFAD = R"(lamp.vert)";
	string FRAGMENT_LAMP_SHADER_PFAD = R"(lamp.frag)";
	string VERTEX_DEPTH_SHADER_PFAD = R"(depth.vert)";
	string FRAGMENT_DEPTH_SHADER_PFAD = R"(depth.frag)";
	string VERTEX_DEPTHDISPLAY_SHADER_PFAD = R"(display_depth.vert)";
	string FRAGMENT_DEPTHDISPLAY_SHADER_PFAD = R"(display_depth.frag)";


	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;

	unsigned int texture1, texture2, texture3;
	unsigned int cubeVAO;
	unsigned int lightVAO;
	unsigned int VBO;
	glm::vec3* cubePositions;
	glm::vec3 bigCubePos;
	glm::vec3 lightPositions;
	glm::vec3 lightFront;

	unsigned mHandleToDepthTexture = 0;
	unsigned mHandleToFramebufferObject = 0;
	unsigned mFBOWidth = 1024;
	unsigned mFBOHeight = 1024;
	float fov;
	float nearPlane;
	float farPlane;

	float deltaTime;
	float lastFrame;
	bool firstMouse;
	float yaw;
	float pitch;
	float lastX;
	float lastY;
	bool isPressedW;
	bool isPressedS;
	bool isPressedA;
	bool isPressedD;
	float cameraSpeed;

	float constantb;
	float slopeScale;
	float rad;
	float samples;
};