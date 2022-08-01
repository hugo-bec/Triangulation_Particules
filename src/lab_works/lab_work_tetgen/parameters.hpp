#ifndef _PARAMETERS_HPP_
#define _PARAMETERS_HPP_

#include "define.hpp"

namespace SIM_PART
{
	extern bool DSTRUCTURE_VERBOSE;
	extern int	  TYPE_SIMULATION;

	extern int	  NB_POINT;
	extern float  SPEED_POINTS;
	extern float  ATTRACT_RADIUS;
	extern float  SIZE_OBJECTS;
	extern int	  TETRA_REFRESH_RATE;

	extern Vec3f  CAGE_DIM;
	extern int	  NB_INIT_FIXED_POINTS;

	//Boids
	extern float BOIDS_SEPARATION_DISTANCE;
	extern float BOIDS_BOX_MARGIN;

	extern float BOIDS_COHESION_FACTOR;
	extern float BOIDS_ALIGNMENT_FACTOR;
	extern float BOIDS_SEPARATION_FACTOR;
	extern float BOIDS_TURN_FACTOR;

	

	
}
	
#endif