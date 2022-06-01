#include "application.hpp"
#include <iostream>

int main( int argc, char * argv[] )
{
	try
	{
		const std::string appTitle = "Simulation particules";
		const int		  width	   = 1280;
		const int		  height   = 720;

		SIM_PART::Application app( appTitle, width, height );

		return app.run();
	}
	catch ( const std::exception & e )
	{
		std::cerr << "Exception caught: " << std::endl << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
