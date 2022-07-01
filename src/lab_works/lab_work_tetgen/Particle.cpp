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
		//return ( dx * dx + dy * dy + dz * dz < attract_distance * attract_distance);
		return sqrt( dx * dx + dy * dy + dz * dz) < attract_distance;
	}

	bool Particle::is_same( Particle * p ) const
	{
		const float * pCoords = p->get_coord();
		return !( pCoords[ 0 ] != _coord[ 0 ] || pCoords[ 1 ] != _coord[ 1 ] || pCoords[ 2 ] != _coord[ 2 ] );
	}
	
	void Particle::tri_voisin()
	{
		sort( _neighbours.begin(), _neighbours.end() );
		auto last = std::unique( _neighbours.begin(), _neighbours.end() );
		_neighbours.erase( last,_neighbours.end() );
	}

	void Particle::compute_neighbours( std::vector<Tetrahedron *> tetra_list )
	{
		bool belongs = false;
		Tetrahedron * tetrahedron;

		for ( int i = 0; i < (int)_tetras.size(); i++ )
		{
			tetrahedron = tetra_list[ _tetras[ i ] ];
			for ( int j = 0; j < 4; j++ )
			{
				belongs = false;
				const std::vector<int> * tetra_points = tetrahedron->get_points();
				if ( _id != (*tetra_points)[ j ] )
				{
					for ( int k = 0; k < (int)_neighbours.size(); k++ )
					{
						if ( this->_neighbours[ k ] == (*tetra_points)[ j ] )
						{
							belongs = true;
							break;
						}
					}

					if ( !belongs )
						_neighbours.emplace_back( (*tetra_points)[ j ] );
				}
			}
		}
	}

	/* void Particle::compute_neighbours_v2( std::vector<Tetrahedron *> tetra_list )
	{

		Tetrahedron * tetrahedron;
		for ( int i = 0; i < (int)_tetras.size(); i++ )
		{
			tetrahedron = tetra_list[ _tetras[ i ] ];
			const std::vector<int> * tetra_points = tetrahedron->get_points();
			for ( int j = 0; j < 4; j++ )
			{
				if ( _id != (*tetra_points)[ j ] )
				{
					_neighbours.emplace_back( (*tetra_points)[ j ] );
				}
			}
		}

		sort( _neighbours.begin(), _neighbours.end() );
		auto last = std::unique( _neighbours.begin(), _neighbours.end() );
		_neighbours.erase( last, _neighbours.end() );
	}*/

	void Particle::compute_point_attract( float r, std::vector<Particle *> pointList )
	{
		std::vector<int> points			 = this->_neighbours;
		std::vector<int> traveled_points = this->_neighbours;
		bool			 belongs		 = false;
		Particle *		 p;

		while ( points.size() != 0 )
		{
			p = pointList[ points[ 0 ] ];
			if ( this->is_attract( p, r ) )
			{
				this->_particules_attract.push_back( points[ 0 ] );
				for ( int i = 0; i < (*p->get_neighbours()).size(); i++ )
				{
					belongs = false;

					for ( int j = 0; j < (int)traveled_points.size(); j++ )
					{
						if ( (*p->get_neighbours())[ i ] == traveled_points[ j ] || _id == (*p->get_neighbours())[ i ] )
						{
							belongs = true;
							break;
						}
					}

					if ( !belongs )
					{
						traveled_points.push_back( (*p->get_neighbours())[ i ] );
						points.push_back( (*p->get_neighbours())[ i ] );
					}
				}
			}

			points.erase( points.begin() );
		}
	}


	void Particle::compute_point_attract_v4( float						  r,
											 const std::vector<Particle *> & pointList,
											 std::vector<int> &				 traveled_point,
											 int							 refresh_mesh )
	{
		for ( int i = 0; i <= _id; i++ )
		{
			traveled_point[ i ] = _id;
		}

		for ( int i = _id + 1; i < traveled_point.size(); i++ )
		{
			traveled_point[ i ] = -1;
		}

		for ( int i = 0; i < (int)_neighbours.size(); i++ ) 
		{
			if (_neighbours[i] > _id) 
			{
				traveled_point[ _neighbours[ i ] ] = _id;
				_particules_attract.emplace_back( _neighbours[ i ] );
			}

		}
		
		Particle * p;
		int		i = 0;

		while (i < _particules_attract.size()) 
		{
			p		= pointList[ _particules_attract[ i ] ];
			if ( p->get_id() > _id )
			{
				float d = this->compute_distance_squared( p );
				float radius_futur = r + 2 * _speed * refresh_mesh;
			
				if ( d <= radius_futur * radius_futur )
				{
					_possible_futur_attract.emplace_back( p->_id );
					p->add_possible_attract( _id );

					if ( is_attract(p, r ))
					{
						p->add_attract( _id );

						const std::vector<int>* p_neighbours = p->get_neighbours();
						for ( int j = 0; j < p_neighbours->size(); j++ )
						{
							if ( traveled_point[ (*p_neighbours)[ j ] ] != _id )
							{
								_particules_attract.emplace_back( (*p_neighbours)[ j ] );
								traveled_point[ (*p_neighbours)[ j ] ] = _id;
							}
							
						}
						i++;
					}

					else
					{
						_particules_attract.erase( _particules_attract.begin() + i );
					}
				}
				else
				{
					_particules_attract.erase( _particules_attract.begin() + i );
				}
			}
			else
			{
				std::vector<int> p_neighbours = ( *p->get_neighbours() );
				for ( int j = 0; j < p_neighbours.size(); j++ )
				{
					if ( traveled_point[ p_neighbours[ j ] ] != _id )
					{
						_particules_attract.emplace_back( p_neighbours[ j ] );
						traveled_point[ p_neighbours[ j ] ] = _id;
					}
				}
				i++;
			}


		}
		_taille_attract = _particules_attract.size();
	}
	
	void Particle::compute_point_attract_parallelisable( float						   r,
														 const std::vector<Particle *> & point_list,
														 int							 refresh_mesh
														 /* std::vector<int>			 traveled_point*/)
	{
		
			for ( int i = 0; i < point_list.size(); i++ )
			{
				if ( i != _id )
				{
					float radius_futur = r + 2 * _speed * refresh_mesh;
					if ( is_attract( point_list[ i ], radius_futur ) )
					{
						_possible_futur_attract.emplace_back( point_list[ i ]->_id );

						if ( is_attract( point_list[ i ], r ) )
						{
							_particules_attract.emplace_back( i );
						}
					}
				}
			}
			
	}

	void Particle::compute_point_attract_parallelisable_v2(  float							 r,
															 const std::vector<Particle *> & pointList,
															std::vector<int> &				traveled_point,
															int								refresh_mesh )
	{

		
		for ( int i = 0; i < (int)_neighbours.size(); i++ )
		{
			if ( _neighbours[ i ] > _id )
			{
				traveled_point[ _neighbours[ i ] ] = _id;
				_particules_attract.emplace_back( _neighbours[ i ] );
			}
		}

		Particle * p;
		int		   i = 0;

		while ( i < _particules_attract.size() )
		{
			p = pointList[ _particules_attract[ i ] ];
			
				float radius_futur = r + 2 * _speed * refresh_mesh;

				if ( is_attract(p, radius_futur ))
				{
					_possible_futur_attract.emplace_back( p->_id );
					
					if ( is_attract( p, r ) )
					{
						const std::vector<int> * p_neighbours = p->get_neighbours();
						for ( int j = 0; j < p_neighbours->size(); j++ )
						{
							if ( traveled_point[ ( *p_neighbours )[ j ] ] != _id )
							{
								_particules_attract.emplace_back( ( *p_neighbours )[ j ] );
								traveled_point[ ( *p_neighbours )[ j ] ] = _id;
							}
						}
						i++;
					}

					else
					{
						_particules_attract.erase( _particules_attract.begin() + i );
					}
				}
				else
				{
					_particules_attract.erase( _particules_attract.begin() + i );
				}
		
		}
	}
    
	void Particle::compute_point_attract_brut( float r, std::vector<Particle *> pointList )
	{
		int nb = 0;
		if ( _particules_attract.size() != 0)
			_particules_attract.clear();
		//possible_futur_attract.clear();
		//std::cout << " Points attract brute :" << std::endl;
		for (int i = 0; i < (int)pointList.size(); i++) 
		{
			if (i!= _id && this->is_attract(pointList[i], r)) {
				//std::cout << pointList[ i ]->getId() << std::endl;
				_particules_attract.push_back( i );
				nb++;
			}
		}
		_taille_attract = _particules_attract.size();
		//std::cout << " Nb Points d'attraction en brute : " <<nb<< std::endl;
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
	void Particle::compute_attract_by_double_radius( const float				  rayon,
													 const std::vector<Particle *> & point_list,
												  std::vector<int> &		   traveled_point,
												  int						   iteration,
												  int						   refresh_frame )
	{
		
		/* if ( _taille_attract != 0 )
		{
			std::cout << "taille attract ancienne : " << _taille_attract << " taille actuelle "
					  << _particules_attract.size() << std::endl;
			_particules_attract.erase( _particules_attract.begin(),
									   ( _particules_attract.begin() + _taille_attract - 1 ) );
		}*/

		for ( int i = 0; i < _particules_attract.size(); i++ ) {
			if ( !is_attract( point_list[ _particules_attract[ i ] ], rayon ) )
				_particules_attract.erase( _particules_attract.begin() + i );
		}

		Particle * p;
		for ( int i = 0; i < _possible_futur_attract.size(); i++ )
		{
			if ( _possible_futur_attract[ i ] > _id )
			{
				p = point_list[ _possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_particules_attract.push_back( p->_id );
					p->add_attract( _id );
				}
			}
		}
		
		
		sort( _particules_attract.begin(), _particules_attract.end() );
		auto last = std::unique( _particules_attract.begin(), _particules_attract.end() );
		_particules_attract.erase( last, _particules_attract.end() );

		_taille_attract = _particules_attract.size();
	}
	

	void Particle::compute_diffusion_limited_aggregation( const float				   rayon, 
												  const std::vector<Particle *> & pointList,
												  std::vector<int>	   &traveled_point,
												  int				   iteration,
												  int				   refresh_frame,
												  int				   nb_non_fix )
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
					if ( _fix || p->is_fix() )
					{
						_fix = true;
						p->set_fix( true );
					}
				}
			}

			sort( _particules_attract.begin(), _particules_attract.end() );
			auto last = std::unique( _particules_attract.begin(), _particules_attract.end() );
			_particules_attract.erase( last, _particules_attract.end() );
		}
		
	}

	void Particle::compute_diffusion_limited_aggregation_V2( const float					 rayon,
															 const std::vector<Particle *> & pointList,
															 std::vector<int> &				 traveled_point,
															 int							 iteration,
															 int							 refresh_frame,
															 int							 nb_non_fix)
	{
		for ( int i = 0; i < _particules_attract.size(); i++ )
		{
			if ( !is_attract( pointList[ _particules_attract[ i ] ], rayon ) )
				_particules_attract.erase( _particules_attract.begin() + i );
		}
		
		if ( nb_non_fix < 5000 )
		{
			if ( !is_fix() )
			{
				//_particules_attract.clear();

				compute_attract_by_double_radius( rayon, pointList, traveled_point, iteration, refresh_frame );

				for ( int i = 0; i < _particules_attract.size(); i++ )
				{
					if ( pointList[ _particules_attract[ i ] ]->is_fix() )
						_fix = true;
				}

				_taille_attract = _particules_attract.size();
			}
		}
		else
		{
			if ( is_fix() )
			{

				for ( int i = 0; i < _particules_attract.size(); i++ )
				{
					pointList[ _particules_attract[ i ] ]->set_fix( true );
						
				}

				_taille_attract = _particules_attract.size();
			}
		}

		sort( _particules_attract.begin(), _particules_attract.end() );
		auto last = std::unique( _particules_attract.begin(), _particules_attract.end() );
		_particules_attract.erase( last, _particules_attract.end() );
		 if ( is_fix() )
		{
			std::cout << _id << " nb point attract : " << _particules_attract.size() << std::endl;
			if (_particules_attract.size() != 0) {
				for (int i = 0; i < _particules_attract.size(); i++) {
					std::cout << _particules_attract[ i ] << std::endl;
				}
			}
			printf( "\n" );
			
		}

	}


	void Particle::compute_attract_by_flooding( const float					 rayon, 
												const std::vector<Particle *> & pointList,
												 std::vector<int>	  &traveled_point,
												 int				  iteration,
												 int				  refresh_frame,
												 int				  degre_voisinage)
	{
		_particules_attract.clear();
		for ( int i = 0; i < traveled_point.size(); i++ )
		{
			traveled_point[ i ] = -1;
		}
		traveled_point[ _id ] = _id;

		for ( int i = 0; i < _neighbours.size(); i++ )
		{
			traveled_point[ _neighbours[ i ]  ] = _id;
		}
			
		std::vector<int> n = _neighbours;
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
				
				std::vector<int> neighbourg_i = (*pointList[ n[ i ] ]->get_neighbours());
				for ( int j = 0; j < neighbourg_i.size(); j++ ) 
				{
					if ( traveled_point[ neighbourg_i[ j ] ] != _id )
						n2.emplace_back( neighbourg_i[ j ] );

				}
			}

			n.clear();
			n = n2;
			n2.clear();
			degre_voisinage--;
		}
	}

	
} // namespace SIM_PART