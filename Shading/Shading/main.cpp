
#include "mysample.h"

#include <iostream>

int main()
{
	if (!glfwInit())
		return -1;

	std::shared_ptr<cg2::SampleWindow> window = cg2::SampleWindow::create<MySample>("Cube Demo", 768, 768, 3, 3, 8);
	if (window == nullptr)
	{
		glfwTerminate();
		return -1;
	}

	while (1)
	{
		if (window->shouldClose())
		{
			window.reset();
			break;
		}

		window->onNewFrame();
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
