#include <GLFW/glfw3.h>


import std;

import app04;
import app05;
import app06;
import lesson.hello_triangle;

int main()
{
	try
	{
		lesson::HelloTriangle app(800, 600);
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
}