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
	prog = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_SHADER_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_SHADER_PFAD) } });
	deferrerdshading = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_SHADER_DEF_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_SHADER_DEF_PFAD) } });
	light = cg2::GlslProgram::create({ { cg2::ShaderType::VERTEX_SHADER, cg2::loadShaderSource(VERTEX_SHADER_LIGHT_PFAD) },{ cg2::ShaderType::FRAGMENT_SHADER, cg2::loadShaderSource(FRAGMENT_SHADER_LIGHT_PFAD) } });
	windowHeight = h;
	windowWidth = w;
}

class MRTRenderPass : public cg2::ModelRenderPass
{
public:
	void operator()(std::shared_ptr<const cg2::Model> model2, std::shared_ptr<cg2::GlslProgram> prog, glm::mat4 const& tf)
	{

		for (auto node : model2->mScenegraph)
		{

			glm::mat4 model = tf * node->mModelTf;
			glm::mat3 normals2world = glm::mat3(glm::inverse(glm::transpose(model)));

			prog->setUniformMat4("model", model);

			for (auto id : node->mMeshIds)
			{
				auto meshData = model2->mMeshDataCPU[id];
				auto vao = model2->mVAOs[id];

				glBindVertexArray(vao);
				glDrawElements(GL_TRIANGLES, meshData.mIndexCount, GL_UNSIGNED_INT, nullptr);
				glBindVertexArray(0);
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};

void MySample::renderFrame()
{
	GLenum status = glGetError();


	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0.0f, 0, 1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	MRTRenderPass writeMRTrenderPass;
	cg2::GlslProgram::setActiveProgram(prog);	
	

	glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f); 
	prog->setUniformMat4("projection", projection, false);
	glm::mat4 view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
	prog->setUniformMat4("view", view, false);	


	for (unsigned int i = 0; i < objectPositions.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		model = glm::mat4(1.0f);
		model = glm::translate(model, objectPositions[i]);
		model = glm::scale(model, glm::vec3(1));
		writeMRTrenderPass(mModel, prog, model);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	cg2::GlslProgram::setActiveProgram(deferrerdshading);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);


	deferrerdshading->setUniformVec3("viewPos", cameraPos);

	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
			deferrerdshading->setUniformVec3("light.Position", lightPositions[i]);
			deferrerdshading->setUniformVec3("light.Color", lightColors[i]);
			const float constant = 1.0; 
			const float linear = 0.7;
			const float quadratic = 1.8;
			deferrerdshading->setUniformVal("light.Linear", linear);
			deferrerdshading->setUniformVal("light.Quadratic", quadratic);
			const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
			float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
			deferrerdshading->setUniformVal("light.Radius", radius);
			renderQuad();

	}
	glDisable(GL_BLEND);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	cg2::GlslProgram::setActiveProgram(light);
	light->setUniformMat4("projection", projection);
	light->setUniformMat4("view", view);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, glm::vec3(0.125f));
		light->setUniformMat4("model", model);
		light->setUniformVec3("lightColor", lightColors[i]);
		
		renderCube();
	}

	if (isPressedW) cameraPos += cameraSpeed * cameraFront;
	else if (isPressedS) cameraPos -= cameraSpeed * cameraFront;
	else if (isPressedA) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	else if (isPressedD) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (status != GL_NO_ERROR)
	{
		std::cout << status << std::endl;
	}



}

void MySample::init()
{
	cg2::GlslProgram::setActiveProgram(prog);

	firstMouse = true;
	yaw = -90.0f;
	pitch = 0.0f;
	lastX = 800.0f / 2.0;
	lastY = 600.0 / 2.0;
	fov = 45.0f;

	cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


	isPressedW = false;
	isPressedA = false;
	isPressedD = false;
	isPressedS = false;

	mModel = cg2::Model::import("sphere_lowres.obj");



	objectPositions.push_back(glm::vec3(0.0, 0.0, 0.0));
	objectPositions.push_back(glm::vec3(1.5, -3.0, -1.5));
	objectPositions.push_back(glm::vec3(1.5, -3.0, 1.5));
	objectPositions.push_back(glm::vec3(1.5, 3.0, -1.5));
	objectPositions.push_back(glm::vec3(1.5, 3.0, 1.5));
	objectPositions.push_back(glm::vec3(-1.5, -3.0, -1.5));
	objectPositions.push_back(glm::vec3(-1.5, -3.0, 1.5));
	objectPositions.push_back(glm::vec3(-1.5, 3.0, -1.5));
	objectPositions.push_back(glm::vec3(-1.5, 3.0, 1.5));
	objectPositions.push_back(glm::vec3(-4, -3.0, -1.5));
	objectPositions.push_back(glm::vec3(-4, -3.0, 1.5));
	objectPositions.push_back(glm::vec3(4, -3.0, -1.5));
	objectPositions.push_back(glm::vec3(4, -3.0, 1.5));
	objectPositions.push_back(glm::vec3(4, 0.0, 0.5));
	objectPositions.push_back(glm::vec3(-4, 0.0, 0.5));


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

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	
	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);


	GLenum status = glGetError();

	if (status != GL_NO_ERROR)
	{
		std::cout << status << "******" << std::endl;
	}


	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	if (status != GL_NO_ERROR)
	{
		std::cout << status << "********" << std::endl;
	}

	srand(13);

	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
		float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

		float rColor = ((rand() % 100) / 200.0f) + 0.5; 
		float gColor = ((rand() % 100) / 200.0f) + 0.5; 
		float bColor = ((rand() % 100) / 200.0f) + 0.5; 
		lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	}

	cg2::GlslProgram::setActiveProgram(deferrerdshading);
	deferrerdshading->setUniformTexVal("gPosition", 0);
	deferrerdshading->setUniformTexVal("gNormal", 1);
	deferrerdshading->setUniformTexVal("gAlbedoSpec", 2);
}


void MySample::renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void MySample::cleanup()
{
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
}

void MySample::renderCube()
{
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			 -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			 -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void MySample::resize(const unsigned w, const unsigned h)
{
	glViewport(0, 0, w, h);
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
	cameraFront = glm::normalize(front);
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