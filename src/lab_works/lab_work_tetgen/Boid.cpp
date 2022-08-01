#include "Boid.hpp"

namespace SIM_PART
{
	/* sources:
	 * http://www.red3d.com/cwr/boids/
	 * http://www.kfish.org/boids/pseudocode.html
	 * https://github.com/beneater/boids/blob/master/boids.js
	 */

	void Boid::update_transformation_matrix_with_rotation()
	{
		update_transformation_matrix();

		// TODO: rotation mesh

		/* Vec3f v_rot = Vec3f( _vel.x, _vel.y, _vel.z );
		v_rot = glm::normalize( v_rot );
		Vec3f v_coord = Vec3f( _coord[ 0 ], _coord[ 1 ], _coord[ 2 ] );

		_model._transformation = glm::lookAt( v_coord, v_rot, Vec3f( 0, 1, 0 ) );
		_model._transformation
			= glm::translate( _model._transformation, Vec3f( _coord[ 0 ], _coord[ 1 ], _coord[ 2 ] ) );
		_model._transformation = glm::scale( _model._transformation, _model_scale );
		_model.printTransformationMatrix();*/

		/*_model._transformation = MAT4F_ID;

		_model._transformation[ 3 ][ 0 ] = 0;
		_model._transformation[ 3 ][ 1 ] = 0;
		_model._transformation[ 3 ][ 2 ] = 0;*/


		//float theta = acos( v_rot.z );
		//float phi	= atan( v_rot.y / v_rot.x );
		//std::cout << "phi: " << phi << ", theta: " << theta << std::endl;
		//if ( phi > PIf / 2 && phi < -PIf/2 ) std::cout << "PI/2 > !" << std::endl;

		//v_rot.x = acos( glm::dot( v_rot, Vec3f( -1.f, 0, 0 ) ) );
		//v_rot.y = acos( glm::dot( v_rot, Vec3f( 0, 1.f, 0 ) ) );
		//v_rot.z = acos( glm::dot( v_rot, Vec3f( 0, 0, 1.f ) ) );

		//v_rot.x = v_rot.z < 0 ? v_rot.x + PIf : v_rot.x;
		//v_rot.x = v_rot.y > 0 ? v_rot.x + PIf : v_rot.x;

		//_model._transformation = glm::rotate( _model._transformation, v_rot.x, Vec3f( 0, 1, 0 ) );
		//_model._transformation = glm::rotate( _model._transformation, v_rot.y, Vec3f( 1, 0, 0 ) );
		//_model._transformation = glm::rotate( _model._transformation, theta, Vec3f( 0, 1, 0 ) );
		//_model._transformation = glm::rotate( _model._transformation, -1.5f, Vec3f( 0, 1, 0 ) );
		//_model._transformation = glm::rotate( _model._transformation, 1.5f, Vec3f( 0, 0, 1 ) );

	}

	void Boid::set_coord( float px, float py, float pz )
	{
		_coord[ 0 ] = px;
		_coord[ 1 ] = py;
		_coord[ 2 ] = pz;
		update_transformation_matrix_with_rotation();
	}


	void Boid::move(float speed, Vec3f dimCage)
	{
		Vec3f v_cohesion, v_alignment, v_separation;

		v_cohesion	 = _cohesion();
		v_alignment	 = _alignment();
		v_separation = _separation();

		_vel += v_cohesion;
		_vel += v_separation;
		_vel += v_alignment;

		_limit_mag_velocity( SPEED_POINTS );
		_manage_collision( dimCage );

		set_coord( _coord[ 0 ] + _vel.x, _coord[ 1 ] + _vel.y, _coord[ 2 ] + _vel.z );
	}

	Vec3f Boid::_cohesion()
	{
		Vec3f ret			  = Vec3f(0);

		if ( !_points_attract.empty() )
		{
			for ( int id_b = 0; id_b < _points_attract.size(); id_b++ )
			{
				//ret.add( b.pos );
				const float * coord_b = _all_boids[ _points_attract[ id_b ] ]->get_coord();
				ret.x += coord_b[ 0 ];
				ret.y += coord_b[ 1 ];
				ret.z += coord_b[ 2 ];
			}

			//ret.div( voisins.size() );
			ret /= _points_attract.size();
			//ret.sub( pos );
			ret.x -= _coord[ 0 ];
			ret.y -= _coord[ 1 ];
			ret.z -= _coord[ 2 ];
			//ret.mult( cohesion_factor );
			ret *= BOIDS_COHESION_FACTOR;
		}
		return ret;
	}

	Vec3f Boid::_alignment()
	{
		Vec3f ret			  = Vec3f( 0 );

		if ( !_points_attract.empty() )
		{
			for ( int id_b = 0; id_b < _points_attract.size(); id_b++ )
			{
				const Vec3f & vel_b = _all_boids[ _points_attract[ id_b ] ]->get_velocity();
				ret += vel_b;
			}
			// ret.div( voisins.size() );
			ret /= _points_attract.size();
			ret -= _vel;
			ret *= BOIDS_ALIGNMENT_FACTOR;
		}
		return ret;
	}

	Vec3f Boid::_separation()
	{
		Vec3f	ret			   = Vec3f( 0 );

		if ( !_points_attract.empty() )
		{
			for ( int id_b = 0; id_b < _points_attract.size(); id_b++ )
			{
				Boid * b = _all_boids[ _points_attract[ id_b ] ];
				const float * coord_b = b->get_coord();
				float		  dx	   = _coord[ 0 ] - coord_b[ 0 ];
				float		  dy	   = _coord[ 1 ] - coord_b[ 1 ];
				float		  dz	   = _coord[ 2 ] - coord_b[ 2 ];
				float  distance = sqrt(dx*dx + dy*dy + dz*dz);

				if ( distance < BOIDS_SEPARATION_DISTANCE )
				{
					//ret.add( new PVector( pos.x - b.pos.x, pos.y - b.pos.y, pos.z - b.pos.z ) );
					ret += Vec3f(	_coord[ 0 ] - coord_b[ 0 ], 
									_coord[ 1 ] - coord_b[ 1 ], 
									_coord[ 2 ] - coord_b[ 2 ]);
				}
			}
			//ret.mult( avoid_factor );
			ret *= BOIDS_SEPARATION_FACTOR;
		}
		return ret;
	}

	void Boid::_manage_collision(Vec3f dim_cage)
	{
		float margin = BOIDS_BOX_MARGIN;
		float turn_factor = BOIDS_TURN_FACTOR;
		if ( _coord[0] < margin )
			_vel.x += turn_factor;
		if ( _coord[ 0 ] > dim_cage.x - margin )
			_vel.x -= turn_factor;
		if ( _coord[ 1 ] < margin )
			_vel.y += turn_factor;
		if ( _coord[ 1 ] > dim_cage.y - margin )
			_vel.y -= turn_factor;
		if ( _coord[ 2 ] < margin )
			_vel.z += turn_factor;
		if ( _coord[ 2 ] > dim_cage.z - margin )
			_vel.z -= turn_factor;
	}

	void Boid::_limit_mag_velocity( float max )
	{
		float mag = glm::length(_vel);
		if ( mag > max ) _vel = ( _vel / mag ) * max;
	}

}