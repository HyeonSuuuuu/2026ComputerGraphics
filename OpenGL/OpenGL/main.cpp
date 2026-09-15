#include <GL/glfw3.h>


import std;
import app02;


int main()
{
	try
	{
		hs::App02 app(800, 600);
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
}