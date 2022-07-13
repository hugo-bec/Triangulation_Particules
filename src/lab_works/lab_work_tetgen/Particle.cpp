#include <cmath>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "GL/gl3w.h"
#include "imgui_impl_glut.h"

#include "Particle.hpp"
#include "define.hpp"
#include "Tetrahedron.hpp"
#include "utils/random.hpp"



namespace SIM_PART
{
	void Particle::add_tetrahedron( Tetrahedron * t )
	{ _tetras.emplace_back( t->get_id() ); }

	void Particle::set_coord( const float px, const float py, const float pz )
	{
		_coord[ 0 ] = px;
		_coord[ 1 ] = py;
		_coord[ 2 ] = pz;
		set_coord_mesh(px, py, pz);
	}

	void Particle::set_coord_mesh(const float px, const float py, const float pz) 
	{
		_model._transformation[ 3 ][ 0 ] = px;
		_model._transformation[ 3 ][ 1 ] = py;
		_model._transformation[ 3 ][ 2 ] = pz;
	}

	bool Particle::is_attract( Particle * p, float attract_distance ) const
	{
		const float * p_coord = p->get_coord();
		float		  dx	  = p_coord[ 0 ] - _coord[ 0 ];
		float		  dy	  = p_coord[ 1 ] - _coord[ 1 ];
		float		  dz	  = p_coord[ 2 ] - _coord[ 2 ];
		return ( dx * dx + dy * dy + dz * dz <= attract_distance * attract_distance);
	}

	void Particle::compute_coloration( int mode_type )
	{
		switch(mode_type)
		{
		case 0:
			if (_attract) set_color( Vec3f(1,0,0) );
			else set_color( Vec3f(0.5) );
			break;
		case 1:
			if ( _fix )
			{
				float lf = _nb_frame_free%1000 / 1000.f;
				float r, g, b;

				float lf_case = lf * 6;
				int	  lf_case_int = int( lf_case );
				float lf_case_reste = lf_case - lf_case_int;

				switch (int(lf_case))
				{
				case 0: r = 1; g = lf_case_reste; b = 0; break;
				case 1: r = 1-lf_case_reste; g = 1; b = 0; break;
				case 2: r = 0; g = 1; b = lf_case_reste; break;
				case 3: r = 0; g = 1-lf_case_reste; b = 1; break;
				case 4: r = lf_case_reste; g = 0; b = 1; break;
				case 5: r = 1; g = 0; b = 1-lf_case_reste; break;
				}

				set_color( Vec3f( r, g, b ) );
				//set_color( Vec3f(0, 1, 0) );
			}
			else set_color( Vec3f(0.5) );
			break;
		}
	}
	
	void Particle::tri_voisin()
	{
		sort( _neighbours.begin(), _neighbours.end() );
		auto last = std::unique( _neighbours.begin(), _neighbours.end() );
		_neighbours.erase( last,_neighbours.end() );
	}


	void Particle::compute_point_attract_parallelisable_brut( float						   r,
														 const std::vector<Particle *> & point_list,
														 int							 refresh_mesh)
	{
		int nb = 0;
		for ( int i = 0; i < point_list.size(); i++ )
		{
			if ( i != _id )
			{
				/* float radius_futur = r + 2 * _speed * refresh_mesh;
				if ( is_attract( point_list[ i ], radius_futur ) )
				{*/
					_possible_futur_attract.emplace_back( point_list[ i ]->_id );

					if ( is_attract( point_list[ i ], r ) )
					{
						//particules_attract.emplace_back( i );
						nb++;
					}
				//}
			}
		}
		std::cout << "nb attract brut " << nb << std::endl;
			
			
	}

	void Particle::compute_point_attract_parallelisable_without_double_radius(float							r,
																			const std::vector<Particle *> & pointList,
																			std::vector<int> &				traveled_point)
	{
		_particules_attract.clear();
		traveled_point[ _id ] = _id;
		for ( int i = 0; i < _neighbours.size(); i++ )
		{
			_particules_attract.emplace_back( _neighbours[ i ] );
			traveled_point[ _neighbours[ i ] ] = _id;
		}


		int i = 0;
		while ( i < _particules_attract.size()) 
		{
			Particle * p = pointList[ _particules_attract[ i ] ];
			if ( is_attract( p, r ) )
			{
				i++;
				for ( int j = 0; j < p->get_neighbours()->size(); j++ )
				{
					if ( traveled_point[ ( *p->get_neighbours() )[ j ] ] != _id )
					{
						_particules_attract.emplace_back( ( *p->get_neighbours() )[ j ] );
						traveled_point[ ( *p->get_neighbours() )[ j ] ] = _id;
					}
				}
			}
			
			else
				_particules_attract.erase( _particules_attract.begin() + i );

			
		}
	}

	void Particle::compute_point_attract_parallelisable_double_radius( float							r,
															const std::vector<Particle *> & pointList,
															std::vector<int> &				traveled_point,
															int								refresh_mesh )
	{
		_particules_attract.clear();
		_possible_futur_attract.clear();
		traveled_point[ _id ] = _id;

		for ( int i = 0; i < _neighbours.size(); i++ )
		{
			_possible_futur_attract.emplace_back( _neighbours[ i ] );
			traveled_point[ _neighbours[ i ] ] = _id;
		}
		float radius_futur = r + 2 * _speed * refresh_mesh;
		int i = 0;
		 while ( i < _possible_futur_attract.size() )
		{
			Particle * p = pointList[ _possible_futur_attract[ i ] ];
			if ( is_attract( p, radius_futur ) )
			{
				i++;
				const std::vector<int>* p_neighbours = p->get_neighbours();
				for ( int j = 0; j < p_neighbours->size(); j++ )
				{
					if ( traveled_point[ ( *p_neighbours )[ j ] ] != _id )
					{
						_possible_futur_attract.emplace_back( ( *p_neighbours )[ j ] );
						traveled_point[ ( *p_neighbours )[ j ] ] = _id;
					}
				}

				if ( is_attract( p, r ) )
					_particules_attract.emplace_back( p->get_id() );
			}

			else
				_possible_futur_attract.erase( _possible_futur_attract.begin() + i );
		}
	}

	float Particle::compute_distance( Particle * point ) const
	{
		const float* p_coord = point->get_coord();
		float dx = p_coord[ 0 ] - _coord[ 0 ];
		float dy = p_coord[ 1 ] - _coord[ 1 ];
		float dz = p_coord[ 2 ] - _coord[ 2 ];
		return sqrt( dx * dx + dy * dy + dz * dz );
	}

	float Particle::compute_distance_squared( Particle * point ) const
	{
		const float * p_coord = point->get_coord();
		float dx = p_coord[ 0 ] - _coord[ 0 ];
		float dy = p_coord[ 1 ] - _coord[ 1 ];
		float dz = p_coord[ 2 ] - _coord[ 2 ];
		return  dx * dx + dy * dy + dz * dz;
		
	}

	void Particle::apply_brownian_mvt( float speed, Vec3f cage_dim ) 
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

			//border management
			if ( tx < 0 || tx > cage_dim.x ) 
				_coord[ 0 ] -= dir.x * 2.f;
			if ( ty < 0 || ty > cage_dim.y )
				_coord[ 1 ] -= dir.y * 2.f;
			if ( tz < 0 || tz > cage_dim.z )
				_coord[ 2 ] -= dir.z * 2.f;

			set_coord_mesh( _coord[ 0 ], _coord[ 1 ], _coord[ 2 ] );
		}
	}

	void Particle::compute_attract_by_double_radius( const float					 rayon,
													 const std::vector<Particle *> & point_list,
													 std::vector<int> &				 traveled_point,
													 int							 iteration,
													 int							 refresh_frame )
	{
		
		
		for ( int i = 0; i < _particules_attract.size(); i++ )
		{
			if ( !is_attract( point_list[ _particules_attract[ i ] ], rayon ) )
			{
				_particules_attract.erase( _particules_attract.begin() + i );
				i--;
			}
		}
	
		Particle * p;
		int possible_futur_attract_size = _possible_futur_attract.size();
		for ( int i = 0; i < possible_futur_attract_size; i++ )
		{
				p = point_list[ _possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_particules_attract.push_back( p->_id );
					
				}
		}

		sort( _particules_attract.begin(), _particules_attract.end() );
		auto last = std::unique( _particules_attract.begin(), _particules_attract.end() );
		_particules_attract.erase( last, _particules_attract.end() );

		
	}
	

	void Particle::compute_diffusion_limited_aggregation( const float				   rayon, 
														  const std::vector<Particle *> & pointList,
														  int					nb_non_fix,
														  int					time_frame)
	{
		if ( ( nb_non_fix <= pointList.size() / 2 && !_fix ) || ( nb_non_fix > pointList.size() / 2 && _fix ) )
		{
			_particules_attract.clear();

			Particle * p;
			for ( int i = 0; i < _possible_futur_attract.size(); i++ )
			{
				p = pointList[ _possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_particules_attract.push_back( p->_id );
					if ( _fix )
						p->set_fix( time_frame );
					else if ( p->is_fix() )
						set_fix( time_frame );
				}
			}

			sort( _particules_attract.begin(), _particules_attract.end() );
			auto last = std::unique( _particules_attract.begin(), _particules_attract.end() );
			_particules_attract.erase( last, _particules_attract.end() );
		}
		
	}



	void Particle::compute_attract_by_flooding( const float					 rayon, 
												const std::vector<Particle *> & pointList,
												 std::vector<int>	  traveled_point,
												 int				  iteration,
												 int				  refresh_frame,
												 int				  degre_voisinage)
	{
		for ( int i = 0; i < _particules_attract.size(); i++ )
		{
			if ( !is_attract( pointList[ _particules_attract[ i ] ], rayon ) )
			{
				_particules_attract.erase( _particules_attract.begin() + i );
				i--;
			}
		}
		traveled_point[ _id ] = _id;

		std::vector<int> n = _particules_attract;
		std::vector<int> n2;
		Particle *		 p;
		while (degre_voisinage != 0) 
		{

			for (int i = 0; i < n.size(); i++) 
			{
				p  = pointList[ n[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_particules_attract.emplace_back( p->get_id() );
				}
				
				const std::vector<int> * tmp = pointList[ n[ i ] ]->get_neighbours();
				for ( int j = 0; j < tmp->size(); j++ ) 
				{
					if ( traveled_point[ (*tmp)[ j ] ] != _id )
					{
						n2.push_back( (*tmp)[ j ] );
						traveled_point[ (*tmp)[ j ] ] = _id;
					}	

				}
			}

			n.clear();
			n = n2;
			n2.clear();
			degre_voisinage--;
		}

		sort( _particules_attract.begin(), _particules_attract.end() );
		auto last = std::unique( _particules_attract.begin(), _particules_attract.end() );
		_particules_attract.erase( last, _particules_attract.end() );
	}

	
	
} // namespace SIM_PART