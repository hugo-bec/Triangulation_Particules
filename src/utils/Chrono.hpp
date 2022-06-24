#pragma once
#include <chrono>
#include <iostream>
#include <string>

namespace SIM_PART
{
	class Chrono
	{
	  public:
		void start() { chrono_start = std::chrono::system_clock::now(); }

		void stop() { chrono_stop = std::chrono::system_clock::now(); }

		void print( std::string message )
		{
			if ( verbose ) {
				std::chrono::duration<double> time = chrono_stop - chrono_start;
				std::cout << message << time.count() << "s" << std::endl;
			}
		}

		void stop_and_print( std::string message )
		{
			stop();
			print( message );
		}

		void set_verbose( bool v ) { verbose = v; }

	  private:
		std::chrono::time_point<std::chrono::system_clock> chrono_start, chrono_stop;
		bool											   verbose = false;

	};
} // namespace SIM_PART
