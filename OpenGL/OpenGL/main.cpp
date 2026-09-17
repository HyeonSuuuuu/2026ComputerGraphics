#include <GL/glfw3.h>


import std;

import app01;
import app02;
import app03;


int main()
{
	try
	{
		hs::App03 app(800, 600);
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
}