#include <string> 
#include <vector>

#include "Particle.hpp"
#include "Tetrahedron.hpp"


namespace SIM_PART
{
	class TetraFileReader
	{
	  public:

		static void readNodes( std::string				 fileName,
							   std::vector<Particle *> & points,
							   TriangleMeshModel &		 origin_model,
							   float					 particle_size,
							   float particle_speed);

		static void readTetras( std::string						fileName,
								std::vector<Particle *> &		points,
								std::vector<Tetrahedron *> &	tetras );
	};
}


