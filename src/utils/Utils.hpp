#pragma once
#include <string>
#include <chrono>
#include <iostream>

namespace SIM_PART
{
	class Utils
	{
	  public:
		static void print_time( std::string										   message,
								std::chrono::time_point<std::chrono::system_clock> start,
								std::chrono::time_point<std::chrono::system_clock> stop )
		{
			std::chrono::duration<double> time = stop - start;
			std::cout << message << time.count() << "s" << std::endl;
		}
	};
}


