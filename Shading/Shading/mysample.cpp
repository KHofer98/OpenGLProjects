#include "mysample.h"
#include "glsl.h"
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "mysample.h"
#include "glsl.h"
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

MySample::MySample(unsigned int w, unsigned int h)
{
	lamp = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_LAMP_SHADER_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_LAMP_SHADER_PFAD) } });
	prog = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_SHADER_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_SHADER_PFAD) } });
	depth = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_DEPTH_SHADER_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_DEPTH_SHADER_PFAD) } });
	renderdepth = cg2::GlslProgram::create({ {cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_DEPTHDISPLAY_SHADER_PFAD)}, {cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_DEPTHDISPLAY_SHADER_PFAD)} });
	windowHeight = h;
	windowWidth = w;
}



void MySample::renderFrame()
{
#pragma region renderdepth

	cg2::GlslProgram::setActiveProgram(depth);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, mFBOWidth, mFBOHeight);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mHandleToFramebufferObject);
	float clearDepth = 1.f;
	glClearBufferfv(GL_DEPTH, 0, std::addressof(clearDepth));

	glm::mat4 viewlight = glm::lookAt(lightPositions, lightPositions + lightFront, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProjection = glm::perspective(glm::radians(fov),1.0f, nearPlane, farPlane);
	depth->setUniformMat4("lightSpace", lightProjection*viewlight, false);

	glBindVertexArray(cubeVAO);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mHandleToDepthTexture);

	glm::mat4 model;
	model = glm::scale(model, glm::vec3(40, 40, 40));
	model = glm::translate(model, bigCubePos);
	depth->setUniformMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	for (unsigned int i = 0; i < 10; i++)
	{
		glm::mat4 model;
		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		depth->setUniformMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	model = glm::mat4(1.0);
	glBindVertexArray(0);
#pragma endregion

	
#pragma region rendercubes

	glActiveTexture(GL_TEXTURE2);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, mHandleToDepthTexture);
	glViewport(0, 0, windowWidth, windowHeight);
	cg2::GlslProgram::setActiveProgram(prog);

	prog->setUniformVec3("spotLight.direction", lightFront);

	prog->setUniformVal("constantbias", constantb);
	prog->setUniformVal("slopeScale", slopeScale);
	prog->setUniformVal("rad", rad);
	prog->setUniformVal("samples", samples);

	
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	prog->setUniformMat4("lightviewprojmat", lightProjection*viewlight, false);
	glActiveTexture(GL_TEXTURE2);
	prog->setUniformTexVal("depth_text", 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture3);

	glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, nearPlane, farPlane);

	prog->setUniformMat4("projection", projection, false);


	glm::mat4 view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
	prog->setUniformMat4("view", view, false);	

	glBindVertexArray(cubeVAO);
	model = glm::scale(model, glm::vec3(20, 20, 20));
	model = glm::translate(model, bigCubePos);
	prog->setUniformMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	for (unsigned int i = 0; i < 10; i++)
	{
		glm::mat4 model;
		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		prog->setUniformMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

#pragma endregion

#pragma region renderlamp

	cg2::GlslProgram::setActiveProgram(lamp);

	lamp->setUniformMat4("view", view, false);
	lamp->setUniformMat4("projection", projection, false);
	glBindVertexArray(lightVAO);
	model = glm::mat4();
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::translate(model, lightPositions);
	lamp->setUniformMat4("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
#pragma endregion

#pragma region renderfromlightsource

	
	glViewport(0, 0, 128, 128);
	glBindVertexArray(cubeVAO);

	cg2::GlslProgram::setActiveProgram(renderdepth);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mHandleToDepthTexture);
	renderdepth->setUniformTexVal("depthTexture", 2);
	renderdepth->setUniformVal("zNear", nearPlane);
	renderdepth->setUniformVal("zFar", farPlane);


	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


#pragma endregion


	
	if (isPressedW) cameraPos += cameraSpeed * cameraFront;
	else if (isPressedS) cameraPos -= cameraSpeed * cameraFront;
	else if (isPressedA) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	else if (isPressedD) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;


	GLenum status = glGetError();

	if (status != GL_NO_ERROR)
	{
		std::cout << status << std::endl;
	}
}


void MySample::init()
{
	GLenum status;
	cg2::GlslProgram::setActiveProgram(prog);
	glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	fov = 45.0f;
	nearPlane = 0.1f;
	farPlane = 100.0f;

	firstMouse = true;
	yaw = -90.0f;
	pitch = 0.0f;
	lastX = 800.0f / 2.0;
	lastY = 600.0 / 2.0;

	cameraPos = glm::vec3(0.0f, 0.0f, 7.0f);
	cameraFront = -cameraPos;
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	isPressedW = false;
	isPressedA = false;
	isPressedS = false;
	isPressedD = false;

#pragma region Positioninit



	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	cubePositions = new glm::vec3[10]{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	bigCubePos = glm::vec3{ 0.0f, 0.0f, 0.0f };
	lightPositions = cameraPos;
	lightFront = -lightPositions;
#pragma endregion

#pragma region containertexture

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenTextures(1, &texture1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width1, height1, nrChannels1;

	unsigned char* data1 = stbi_load("container2.png", &width1, &height1, &nrChannels1, 0);
	if (data1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data1);
#pragma endregion

#pragma region specularTexture

	glGenTextures(1, &texture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char* data2 = stbi_load("container2_specular.png", &width1, &height1, &nrChannels1, 0);
	if (data2)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data2);
	prog->setUniformVec3("viewPos", cameraPos);
	prog->setUniformTexVal("material.diffuse", 0);

	prog->setUniformTexVal("material.specular", 0);
	prog->setUniformVal("material.shininess", 32.0f);
#pragma endregion

#pragma region depthtexture
	glGenTextures(1, std::addressof(mHandleToDepthTexture));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mHandleToDepthTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, mFBOWidth, mFBOHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, std::addressof(mHandleToFramebufferObject));
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mHandleToFramebufferObject);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mHandleToDepthTexture, 0);

	std::vector<GLenum> drawBuffers = { GL_NONE };
	glDrawBuffers(1, drawBuffers.data());

	GLenum status2 = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status2 != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "incomplete framebuffer object " << status2 << std::endl;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

#pragma endregion

#pragma region flashlight
	glGenTextures(1, &texture3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture3);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	int width3, height3, nrChannels3;

	unsigned char* data3 = stbi_load("flashlight.png", &width3, &height3, &nrChannels3, 0);
	if (data3)
	{

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, data3);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}



	stbi_image_free(data3);
	prog->setUniformTexVal("flashlight", 3);
	status = glGetError();

	if (status != GL_NO_ERROR)
	{
		std::cout << status << std::endl;
	}

#pragma endregion
	constantb = 0.00001;
	slopeScale = 0.001;
	rad = 1.0f;
	samples = 64.0f;


#pragma region spotLightInit
	// spotLight
	prog->setUniformVec3("spotLight.position", lightPositions);
	prog->setUniformVec3("spotLight.direction", lightFront);
	prog->setUniformVec3("spotLight.ambient", 3.0f, 3.0f, 3.0f);
	prog->setUniformVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	prog->setUniformVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	prog->setUniformVal("spotLight.constant", 1.0f);
	prog->setUniformVal("spotLight.linear", 0.09);
	prog->setUniformVal("spotLight.quadratic", 0.032);
	prog->setUniformVal("spotLight.cutOff", glm::cos(glm::radians(5.5f)));
	prog->setUniformVal("spotLight.outerCutOff", glm::cos(glm::radians(7.0f)));
#pragma endregion
}


void MySample::cleanup()
{
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
}

void MySample::resize(const unsigned w, const unsigned h)
{
	//glViewport(0, 0, w, h);
}

void MySample::mouseButton(const int button, const int action, const int mods)
{

}

void MySample::mousePosition(const double xpos, const double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lightFront = glm::normalize(front);
}

void MySample::keyInput(const int key, const int scancode, const int action, const int mods)
{
	cameraSpeed = 2.5  *deltaTime;
	if (key == GLFW_KEY_ESCAPE)
	{
		auto window = glfwGetCurrentContext();
		glfwSetWindowShouldClose(window, 1);
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS) isPressedW = true;
	if (key == GLFW_KEY_W && action == GLFW_RELEASE) isPressedW = false;

	if (key == GLFW_KEY_S && action == GLFW_PRESS) isPressedS = true;
	if (key == GLFW_KEY_S && action == GLFW_RELEASE) isPressedS = false;

	if (key == GLFW_KEY_A && action == GLFW_PRESS) isPressedA = true;
	if (key == GLFW_KEY_A && action == GLFW_RELEASE) isPressedA = false;

	if (key == GLFW_KEY_D && action == GLFW_PRESS) isPressedD = true;
	if (key == GLFW_KEY_D && action == GLFW_RELEASE) isPressedD = false;

	if (key == GLFW_KEY_UP && action == GLFW_PRESS) constantb *= 2;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) constantb /= 2;

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) slopeScale *= 2;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) slopeScale /= 2;

	if (key == GLFW_KEY_K && action == GLFW_PRESS) rad += 1.0f;
	if (key == GLFW_KEY_I && action == GLFW_PRESS) rad -= 1.0f;

	if (key == GLFW_KEY_L && action == GLFW_PRESS) samples += 2;
	if (key == GLFW_KEY_O && action == GLFW_PRESS) samples -= 2;
}

void MySample::mouseScroll(const double xoffset, const double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}