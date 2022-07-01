#include "parameters.hpp"

namespace SIM_PART
{
	int NB_PARTICULES			= 50000;
	float SPEED_PARTICULES		= 0.1f;
	float ATTRACT_RADIUS		= 1.f;
	float SIZE_PARTICLE			= 0.2f;
	int	  TETRA_REFRESH_RATE	= 100;

	Vec3f CAGE_DIM				= Vec3f( 10 );
	int	 NB_INIT_FIXED_POINTS	= 50;

	bool DSTRUCTURE_VERBOSE = true;
}