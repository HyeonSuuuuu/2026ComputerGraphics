#include <GLFW/glfw3.h>


import std;

import app04;
import app05;
import app06;
import app07;
import app08;
import app09;


int main()
{
	try
	{
		app06::App app(800, 600);
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
}