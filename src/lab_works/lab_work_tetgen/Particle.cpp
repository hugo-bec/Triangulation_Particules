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
	}

	bool Particle::is_attract( Particle * p, float attract_distance ) const
	{
		const float * p_coord = p->get_coord();
		float		  dx	  = p_coord[ 0 ] - _coord[ 0 ];
		float		  dy	  = p_coord[ 1 ] - _coord[ 1 ];
		float		  dz	  = p_coord[ 2 ] - _coord[ 2 ];
		return ( dx * dx + dy * dy + dz * dz < attract_distance * attract_distance);
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

	void Particle::compute_neighbours_v2( std::vector<Tetrahedron *> tetra_list )
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
	}

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
									   std::vector<int> &traveled_point,
									   int refresh_frame )
	{

		std::chrono::time_point<std::chrono::system_clock> start_init_traveled_points, stop_init_traveled_points,
			start_comparaison, stop_comparaison,  start_parcours_voisin, stop_parcours_voisin;
		std::chrono::duration<double> time_init, time_parcours, time_comparaison;

		start_init_traveled_points	= std::chrono::system_clock::now();
	
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
		//point_attract			  = neighbours;
		stop_init_traveled_points = std::chrono::system_clock::now();
		time_init				  += stop_init_traveled_points - start_init_traveled_points;
		Particle * p;
		int		i = 0;

		while (i < _particules_attract.size()) 
		{
			p		= pointList[ _particules_attract[ i ] ];
			if ( p->get_id() > _id )
			{
				float d = this->compute_distance_squared( p );
				float radius_futur = r + 2 * _speed * refresh_frame;
			
				if ( d <= radius_futur * radius_futur )
				{
					_possible_futur_attract.emplace_back( p->_id );
					p->add_possible_attract( _id );

					if ( d <= r * r )
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
		/* std::vector<int> points = neighbours;
		Point*			 p;
		while ( points.size() != 0 )
		{
			p = pointList[ points[ 0 ] ];
			//p				  = pointList[ points.pop_back() ];
			start_comparaison = std::chrono::system_clock::now();
			float d			  = this->getDistance( p );
			if ( p->getId() > id )
			{
				if ( d <= r + 2 * speed * refresh_frame )
				{
					possible_futur_attract.emplace_back( p->id );
					p->addPossibleAttract( id );
					if ( d <= r )
					{
						stop_comparaison = std::chrono::system_clock::now();
						time_comparaison += stop_comparaison - start_comparaison;
						this->point_attract.emplace_back( p->id );
						p->addAttract( id );
						start_parcours_voisin		 = std::chrono::system_clock::now();
						const std::vector<int>* p_neigbours = p->getNeighbours();
						for ( int i = 0; i < (*p_neigbours).size(); i++ )
						{
							if ( traveled_point[ (*p_neigbours)[ i ] ] != this->id )
							{
								points.push_back( (*p_neigbours)[ i ] );
								traveled_point[ (*p_neigbours)[ i ] ] = this->id;
							}
						}
						stop_parcours_voisin = std::chrono::system_clock::now();
						time_parcours += stop_parcours_voisin - start_parcours_voisin;
					}
				}
			}

			else
			{
				if ( d <= r ) {
					const std::vector<int>* p_neigbours = p->getNeighbours();
					for ( int i = 0; i < (*p_neigbours).size(); i++ )
					{
						if ( traveled_point[ (*p_neigbours)[ i ] ] != this->id )
						{
							points.push_back( (*p_neigbours)[ i ] );
							traveled_point[ (*p_neigbours)[ i ] ] = this->id;
						}
					}
				}
			}
			points.erase( points.begin() );
			
		}*/

		//std::cout << " Fonction computeAttractPoint : " << std::endl;
		//std::cout << " Temps initialisation traveled point : " << time_init.count() << " s" << std::endl;
		//std::cout << " Temps initialisation comparaison : " << time_comparaison.count() << " s" << std::endl;
		//std::cout << " Temps initialisation parcours voisin : " << time_parcours.count() << " s" << std::endl;
	}
	
    
	void Particle::compute_point_attract_brut( float r, std::vector<Particle *> pointList ) const
	{
		int nb = 0;
		//possible_futur_attract.clear();
		//std::cout << " Points attract brute :" << std::endl;
		for (int i = 0; i < (int)pointList.size(); i++) 
		{
			if (i!= _id && this->is_attract(pointList[i], r)) {
				//std::cout << pointList[ i ]->getId() << std::endl;
				//possible_futur_attract.emplace_back( i );
				nb++;
			}
		}
		std::cout << " Nb Points d'attraction en brute : " <<nb<< std::endl;
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
		}
	}
	void Particle::compute_attract_by_double_radius( const float				  rayon,
													 const std::vector<Particle *> & point_list,
												  std::vector<int> &		   traveled_point,
												  int						   iteration,
												  int						   refresh_frame )
	{
		_particules_attract.erase( _particules_attract.begin(), ( _particules_attract.begin() + _taille_attract - 1 ) );
		Particle * p;
		for ( int i = 0; i < _possible_futur_attract.size(); i++ )
		{
			if ( _possible_futur_attract[ i ] > _id )
			{
				p = point_list[ _possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->_particules_attract.emplace_back( p->_id );
					p->add_attract( _id );
				}
			}
		}
		_taille_attract = _particules_attract.size();
	}
	//===============test==================


	void Particle::compute_diffusion_limited_aggregation( const float				   rayon, 
												  const std::vector<Particle *> & pointList,
												  std::vector<int>	   &traveled_point,
												  int				   iteration,
												  int				   refresh_frame )
	{
		for ( int i = 0; i < _particules_attract.size(); i++ )
		{
			if ( pointList[ _particules_attract[ i ] ]->is_fix() )
			{
				_fix = true;
				_color = Vec3f( 0, 1, 0 );

			}
		}

		
			if ( _taille_attract != 0 )
			{
				_particules_attract.erase( _particules_attract.begin(), ( _particules_attract.begin() + _taille_attract - 1 ) );
			}
			for (int i = 0; i < _particules_attract.size(); i++) 
			{
				if ( pointList[ _particules_attract[ i ] ]->is_fix() )
					_fix = true;
			}
			Particle * p;
			for ( int i = 0; i < _possible_futur_attract.size(); i++ )
			{
				if ( _possible_futur_attract[ i ] > _id )
				{	
					p = pointList[ _possible_futur_attract[ i ] ];
					if ( this->is_attract( p, rayon ) )
					{
						this->_particules_attract.push_back( p->_id );
						//std::cout << "est fix " << p->getPointAttract()->size() << std::endl << std::flush;
						p->add_attract( _id );
						if ( !_fix )
						{
							if ( p->is_fix() == true )
							{
								// std::cout << "FIXED !: " << this->id << std::endl;
								_fix = true;
								_color = Vec3f( 0, 1, 0 );
							}
						}
					}
				}
			}
			_taille_attract = _particules_attract.size();
		

		/* else
		{
			if ( taille_attract != 0 )
			{
				point_attract.erase( point_attract.begin(), ( point_attract.begin() + taille_attract - 1 ) );
			}
			Particle * p;
			for ( int i = 0; i < possible_futur_attract.size(); i++ )
			{
				if ( possible_futur_attract[ i ] > id )
				{
					p = pointList[ possible_futur_attract[ i ] ];
					if ( this->is_attract( p, rayon ) )
					{
						this->point_attract.push_back( p->id );
						// std::cout << "est fix " << p->getPointAttract()->size() << std::endl << std::flush;
						p->addAttract( id );
						p->set_fix(true);
						
					
					}
				}
			}
			taille_attract = point_attract.size();
		}*/
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