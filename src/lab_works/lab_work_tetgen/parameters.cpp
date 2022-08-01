#include "parameters.hpp"

namespace SIM_PART
{
	bool DSTRUCTURE_VERBOSE = false;
	// 0: particules, 1: boids
	int TYPE_SIMULATION = 1;

	int NB_PARTICULES			= 10000;
	float SPEED_PARTICULES		= 0.1f;
	float ATTRACT_RADIUS		= 1.f;
	float SIZE_PARTICLE			= 0.1f;
	int	  TETRA_REFRESH_RATE	= 5;

	Vec3f CAGE_DIM				= Vec3f( 10 );
	int	 NB_INIT_FIXED_POINTS	= 0;

	//Boids
	float SPEED_BOIDS = 0.1;

	
}