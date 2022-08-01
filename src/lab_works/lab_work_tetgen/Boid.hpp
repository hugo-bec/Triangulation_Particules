#pragma once
#include "Point.hpp"
#include "utils/random.hpp"

namespace SIM_PART
{
	class Boid : public Point
	{
	  public:
		Boid( int p_id, float p_x, float p_y, float p_z, TriangleMeshModel & p_model, 
			float p_size, float p_speed, std::vector<Point *> & p_all_boids ) :
			Point( p_id, p_x, p_y, p_z, p_model, p_size, p_speed ),
			_all_boids( (std::vector<Boid *> &)p_all_boids )
		{
			_vel = Vec3f( getRandomFloat()-0.5, getRandomFloat()-0.5, getRandomFloat()-0.5 );
			glm::normalize(_vel);
			_vel *= SPEED_POINTS;
		}

		const Vec3f & get_velocity() const { return _vel; }
		float		  signe( float f ) { return f < 0 ? -1 : f > 0 ? 1 : 0; }

		void set_coord( float px, float py, float pz ) override;

		void update_transformation_matrix_with_rotation();

		void move( float speed, Vec3f dimCage ) override;

	  private:

		Vec3f _cohesion();
		Vec3f _alignment();
		Vec3f _separation();
		void  _manage_collision( Vec3f dim_cage );
		void  _limit_mag_velocity( float max );


		Vec3f _vel;	//velocity
		std::vector<Boid *> & _all_boids;
	};
} // namespace SIM_PART
