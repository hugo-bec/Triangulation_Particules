#ifndef _PARAMETERS_HPP_
#define _PARAMETERS_HPP_

#include "define.hpp"

namespace SIM_PART
{
	extern bool DSTRUCTURE_VERBOSE;
	extern int	  TYPE_SIMULATION;

	extern int	  NB_PARTICULES;
	extern float  SPEED_PARTICULES;
	extern float  ATTRACT_RADIUS;
	extern float  SIZE_PARTICLE;
	extern int	  TETRA_REFRESH_RATE;

	extern Vec3f  CAGE_DIM;
	extern int	  NB_INIT_FIXED_POINTS;

	//Boids
	extern float SPEED_BOIDS;

	
}
	
#endif