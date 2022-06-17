#include <string> 
#include <vector>

#include "Point.hpp"
#include "Tetrahedron.hpp"


namespace SIM_PART
{
	class TetraFileReader
	{
	  public:

		static void readNodes( std::string fileName, std::vector<Point*> & points );

		static void readTetras( std::string						fileName,
								std::vector<Point *> &			points,
								std::vector<Tetrahedron *> &	tetras );
	};
}


