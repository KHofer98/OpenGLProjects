#pragma once
/*
provided by DI Gerlinde Emsenhuber
MultiMediaTechnology
FH Salzburg
*/


#include <algorithm>
#include <memory>
#include <string>
#include<iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace cg2
{
	class Sample
	{

	public:

		// should contain all rendering related code, as well as scene updates.
		virtual void renderFrame() = 0;

		// should all contain initialization code -- file i/o & especially OpenGL initialization (texture & vertex data upload @GPU)
		// when this function is called, you are guaranteed to have a valid GLFW window & associated OpenGL context
		virtual void init() = 0;

		// should free all ressources. once this function has been called, the GLFW windows & associated OpenGL context will be destroyed.
		virtual void cleanup() = 0;

		// glfw callbacks -- if not needed, just leave the method empty
		// you will always need to handle resize events!
		virtual void resize(const unsigned w, const unsigned h) = 0;
		virtual void mouseButton(const int button, const int action, const int mods) = 0;
		virtual void mousePosition(const double xpos, const double ypos) = 0;
		virtual void keyInput(const int key, const int scancode, const int action, const int mods) = 0;
		virtual void mouseScroll(const double xoffset, const double yoffset) = 0;

		Sample(Sample const&) = delete;
		Sample& operator=(Sample const&) = delete;
		Sample(Sample &&) = delete;
		Sample& operator=(Sample &&) = delete;
		Sample() = default;
		virtual ~Sample() = default;

	};

	class SampleWindow
	{

	public:

		static void windowResizeCallback(GLFWwindow *window, int w, int h)
		{
			if (w <= 0 || h <= 0)
				return;

			Sample *app = static_cast<Sample *>(glfwGetWindowUserPointer(window));
			app->resize(w, h);
		}

		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			Sample *app = static_cast<Sample *>(glfwGetWindowUserPointer(window));
			app->mouseButton(button, action, mods);
		}

		static void mousePositionCallback(GLFWwindow* window, double xpos, double ypos)
		{
			Sample *app = static_cast<Sample *>(glfwGetWindowUserPointer(window));
			app->mousePosition(xpos, ypos);
		}

		static void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Sample *app = static_cast<Sample *>(glfwGetWindowUserPointer(window));
			app->keyInput(key, scancode, action, mods);
		}

		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
		{
			Sample *app = static_cast<Sample *>(glfwGetWindowUserPointer(window));
			app->mouseScroll(xoffset, yoffset);
		}

		template <typename SampleDerived>
		static std::shared_ptr<SampleWindow> create(const std::string title, const unsigned width, const unsigned height, const unsigned major, const unsigned minor, const unsigned samples)
		{
			if (width == 0 || height == 0)
			{
				std::cout << "invalid window dimensions" << std::endl;
				return nullptr;
			}

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, samples);

			GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
			if (!window)
			{
				std::cout << "failed to create GLFW window" << std::endl;
				return nullptr;
			}
			glfwMakeContextCurrent(window);
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				std::cout << "failed to initialize GLAD" << std::endl;
				return nullptr;
			}




			int w = 0; int h = 0;
			glfwGetWindowSize(window, &w, &h);
			if (w <= 0 || h <= 0)
			{
				std::cout << "sth. went wrong, glfw window has invalid size " << w << " " << h << std::endl;
				glfwDestroyWindow(window);
				return nullptr;
			}

			std::shared_ptr<SampleWindow> result(new SampleWindow(), SampleWindow::Deleter());

			Sample *app = new SampleDerived(w, h);
			app->init();

			glfwSetWindowUserPointer(window, app);
			glfwSetFramebufferSizeCallback(window, SampleWindow::windowResizeCallback);
			glfwSetMouseButtonCallback(window, SampleWindow::mouseButtonCallback);
			glfwSetCursorPosCallback(window, SampleWindow::mousePositionCallback);
			glfwSetKeyCallback(window, SampleWindow::keyInputCallback);
			glfwSetScrollCallback(window, SampleWindow::scrollCallback);

			result->mSample = app;
			result->mWindow = window;

			return result;
		}

		void onNewFrame() const
		{
			glfwMakeContextCurrent(mWindow);
			mSample->renderFrame();
			glfwSwapBuffers(mWindow);
		}

		bool shouldClose() const
		{
			return glfwWindowShouldClose(mWindow);
		}

		SampleWindow(SampleWindow const&) = delete;
		SampleWindow& operator=(SampleWindow const&) = delete;
		SampleWindow(SampleWindow &&) = delete;
		SampleWindow& operator=(SampleWindow &&) = delete;

	private:

		SampleWindow() = default;
		~SampleWindow() = default;

		struct Deleter
		{
			void operator()(SampleWindow *& p)
			{
				// cleanup needs current context
				glfwMakeContextCurrent(p->mWindow);
				p->mSample->cleanup();

				// destroy ogl sample
				delete p->mSample;
				p->mSample = nullptr;

				// destroy glfw window
				glfwSetWindowUserPointer(p->mWindow, nullptr);
				glfwDestroyWindow(p->mWindow);
				p->mWindow = nullptr;

				// finally, destroy the object itself
				delete p;
				p = nullptr;
			}
		};

		GLFWwindow	* mWindow = nullptr;
		Sample		* mSample = nullptr;

	};
}
