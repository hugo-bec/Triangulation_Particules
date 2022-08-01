#pragma once
#include "lab_work_tetgen/Point.hpp"

namespace SIM_PART
{
	class Particle : public Point
	{
	  public:
		Particle( int p_id, float p_x, float p_y, float p_z, TriangleMeshModel & p_model, float p_size, float p_speed ) :
			Point( p_id, p_x, p_y, p_z, p_model, p_size, p_speed )
		{
		}

		void move( float speed, Vec3f dimCage ) override;

	};
} // namespace SIM_PART