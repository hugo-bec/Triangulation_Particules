#include "parameters.hpp"

namespace SIM_PART
{
	bool DSTRUCTURE_VERBOSE = false;
	int TYPE_SIMULATION = 1;	// 0: particules, 1: boids

	int NB_POINT			= 10000;
	float SPEED_POINTS		= 0.1f;		//speed of particules or boids
	float ATTRACT_RADIUS		= 1.f;	//radius of interaction of particules or boids
	float SIZE_OBJECTS			= 0.1f;	//graphical size of objects
	int	  TETRA_REFRESH_RATE	= 5;

	Vec3f CAGE_DIM				= Vec3f( 10 );
	int	 NB_INIT_FIXED_POINTS	= 0;

	//Boids
	float BOIDS_SEPARATION_DISTANCE = 0.1f;
	float BOIDS_BOX_MARGIN = 0.1;

	float BOIDS_COHESION_FACTOR = 0.005;	//default: 0.005
	float BOIDS_ALIGNMENT_FACTOR = 0.005;	//default: 0.005
	float BOIDS_SEPARATION_FACTOR = 0.05;	//default: 0.05
	float BOIDS_TURN_FACTOR = 0.05;			//default: 0.05
	
	
}