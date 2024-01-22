#pragma comment (lib, "ws2_32.lib")

#include "Server.h"
#include <iostream>
#include <exception>

int main()
{
	try
	{
		Server().serve(6969);
	}
	catch (std::exception& e)
	{
		std::cout << "Error occured: " << e.what() << std::endl;
	}
	return 0;
}
