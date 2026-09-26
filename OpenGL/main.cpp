#include <GLFW/glfw3.h>


import std;

import app04;
import app05;
import app06;

int main()
{
	try
	{
		app06::App06 app(800, 600);
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
}