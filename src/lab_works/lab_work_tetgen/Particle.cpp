#include "Particle.hpp"
#include "utils/random.hpp"

namespace SIM_PART
{
	//apply pseudo-brownian movement
	void Particle::move( float speed, Vec3f cage_dim )
	{
		if ( !_fix )
		{
			SIM_PART::Vec3f dir;
			dir.x = ( ( getRandomFloat() * 2.f ) - 1.f ) * speed;
			dir.y = ( ( getRandomFloat() * 2.f ) - 1.f ) * speed;
			dir.z = ( ( getRandomFloat() * 2.f ) - 1.f ) * speed;

			float tx = _coord[ 0 ] += dir.x;
			float ty = _coord[ 1 ] += dir.y;
			float tz = _coord[ 2 ] += dir.z;

			// border management
			if ( tx < 0 || tx > cage_dim.x )
				_coord[ 0 ] -= dir.x * 2.f;
			if ( ty < 0 || ty > cage_dim.y )
				_coord[ 1 ] -= dir.y * 2.f;
			if ( tz < 0 || tz > cage_dim.z )
				_coord[ 2 ] -= dir.z * 2.f;

			update_transformation_matrix();
		}
	}
}