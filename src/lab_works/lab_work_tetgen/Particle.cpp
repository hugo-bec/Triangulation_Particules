#include <cmath>
#include <cstdio>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "GL/gl3w.h"
#include "imgui_impl_glut.h"

#include "Particle.hpp"
#include "Tetrahedron.hpp"
#include "define.hpp"
#include "utils/random.hpp"



namespace SIM_PART
{


	const float* Particle::get_coord() const { return coord; }

	int Particle::get_id() { return this->id; }

	void Particle::add_tetrahedron( Tetrahedron * t ) { this->tetra.emplace_back( t->get_id() ); }

	const std::vector<int> * Particle::get_tetrahedron() { return &tetra; }

	const std::vector<int> * Particle::get_point_attract() { return &point_attract; }

	const std::vector<int> * Particle::get_neighbours() { return &neighbours; }

	bool Particle::is_attract( Particle * p, float attract_distance )
	{
		const float * p_coord = p->get_coord();
		float dx = p_coord[ 0 ] - x;
		float dy = p_coord[ 1 ] - y;
		float dz = p_coord[ 2 ] - z;
		return ( dx * dx + dy * dy + dz * dz < attract_distance * attract_distance);
	}

	void Particle::set_coord( const float px, const float py, const float pz ) 
	{ 
		x = px;
		y = py;
		z = pz;
		coord[ 0 ] = px;
		coord[ 1 ] = py;
		coord[ 2 ] = pz;
	}

	void Particle::add_point( Particle * p ) { point_attract.emplace_back( p->get_id() ); }

	bool Particle::is_same( Particle * p )
	{
		const float * pCoords = p->get_coord();
		return !( pCoords[ 0 ] != this->x || pCoords[ 1 ] != this->y || pCoords[ 2 ] != this->z );
	}
	
	void Particle::tri_voisin()
	{
		sort( neighbours.begin(), neighbours.end() );
		auto last = std::unique( neighbours.begin(), neighbours.end() );
		neighbours.erase( last,neighbours.end() );
	}

	void Particle::compute_neighbours( std::vector<Tetrahedron *> tetraList )
	{
		bool belongs = false;
		Tetrahedron * tetrahedron;

		for ( int i = 0; i < (int)tetra.size(); i++ )
		{
			tetrahedron = find_tetra( tetraList, tetra[ i ] );
			for ( int j = 0; j < 4; j++ )
			{
				belongs = false;
				const std::vector<int> * tetra_points = tetrahedron->get_points();
				if ( this->id != (*tetra_points)[ j ] )
				{
					for ( int k = 0; k < (int)neighbours.size(); k++ )
					{
						if ( this->neighbours[ k ] == (*tetra_points)[ j ] )
						{
							belongs = true;
							break;
						}
					}

					if ( !belongs )
						neighbours.emplace_back( (*tetra_points)[ j ] );
				}
			}
		}
	}

	void Particle::compute_neighbours_v2( std::vector<Tetrahedron *> tetraList )
	{

		Tetrahedron * tetrahedron;
		for ( int i = 0; i < (int)tetra.size(); i++ )
		{
			tetrahedron = find_tetra( tetraList, tetra[ i ] );
			const std::vector<int> * tetra_points = tetrahedron->get_points();
			for ( int j = 0; j < 4; j++ )
			{
				if ( this->id != (*tetra_points)[ j ] )
				{
					neighbours.emplace_back( (*tetra_points)[ j ] );
				}
			}
		}

		sort( neighbours.begin(), neighbours.end() );
		auto last = std::unique( neighbours.begin(), neighbours.end() );
		neighbours.erase( last, neighbours.end() );
	}


	Particle * Particle::find_point( std::vector<Particle *> pointList, int id ) { return pointList[ id ]; }

	Tetrahedron * Particle::find_tetra( std::vector<Tetrahedron *> tetraList, int id ) { return tetraList[ id ]; }

	void Particle::compute_point_attract( float r, std::vector<Particle *> pointList )
	{
		std::vector<int> points			 = this->neighbours;
		std::vector<int> traveled_points = this->neighbours;
		bool			 belongs		 = false;
		Particle *		 p;

		while ( points.size() != 0 )
		{
			p = find_point( pointList, points[ 0 ] );
			if ( this->is_attract( p, r ) )
			{
				this->point_attract.push_back( points[ 0 ] );
				for ( int i = 0; i < (*p->get_neighbours()).size(); i++ )
				{
					belongs = false;

					for ( int j = 0; j < (int)traveled_points.size(); j++ )
					{
						if ( (*p->get_neighbours())[ i ] == traveled_points[ j ] || this->id == (*p->get_neighbours())[ i ] )
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
	
		for ( int i = 0; i <= id; i++ )
		{
				traveled_point[ i ] = id;
		}
		for ( int i = id + 1; i < traveled_point.size(); i++ )
		{
			traveled_point[ i ] = -1;
		}
		for ( int i = 0; i < (int)neighbours.size(); i++ ) 
		{
			if (neighbours[i] > id) 
			{
				traveled_point[ neighbours[ i ] ] = this->id;
				point_attract.emplace_back( neighbours[ i ] );
			}

		}
		//point_attract			  = neighbours;
		stop_init_traveled_points = std::chrono::system_clock::now();
		time_init				  += stop_init_traveled_points - start_init_traveled_points;
		Particle * p;
		int		i = 0;

		while (i < point_attract.size()) 
		{
			p		= pointList[ point_attract[ i ] ];
			if ( p->get_id() > id )
			{
				float d = this->compute_distance_squared( p );
				float radius_futur = r + 2 * speed * refresh_frame;
			
				if ( d <= radius_futur * radius_futur )
				{
					possible_futur_attract.emplace_back( p->id );
					p->addPossibleAttract( id );

					if ( d <= r * r )
					{
						p->addAttract( id );

						const std::vector<int>* p_neighbours = p->get_neighbours();
						for ( int j = 0; j < p_neighbours->size(); j++ )
						{
							if ( traveled_point[ (*p_neighbours)[ j ] ] != this->id )
							{
								point_attract.emplace_back( (*p_neighbours)[ j ] );
								traveled_point[ (*p_neighbours)[ j ] ] = this->id;
							}
							
						}
						i++;
					}

					else
					{
						point_attract.erase( point_attract.begin() + i );
					}
				}
				else
				{
					point_attract.erase( point_attract.begin() + i );
				}
			}
			else
			{
				std::vector<int> p_neighbours = ( *p->get_neighbours() );
				for ( int j = 0; j < p_neighbours.size(); j++ )
				{
					if ( traveled_point[ p_neighbours[ j ] ] != this->id )
					{
						point_attract.emplace_back( p_neighbours[ j ] );
						traveled_point[ p_neighbours[ j ] ] = this->id;
					}
				}
				i++;
			}


		}
		taille_attract = point_attract.size();
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
	
    
	void Particle::compute_point_attract_brut( float r, std::vector<Particle *> pointList )
	{
		int nb = 0;
		//possible_futur_attract.clear();
		//std::cout << " Points attract brute :" << std::endl;
		for (int i = 0; i < (int)pointList.size(); i++) 
		{
			if (i!= this->id && this->is_attract(pointList[i], r)) {
				//std::cout << pointList[ i ]->getId() << std::endl;
				//possible_futur_attract.emplace_back( i );
				nb++;
			}
		}
		std::cout << " Nb Points d'attraction en brute : " <<nb<< std::endl;
	}

	float Particle::compute_distance( Particle * point ) 
	{
		const float* p_coord = point->get_coord();

		float x = p_coord[ 0 ] - x;
		float y = p_coord[ 1 ] - y;
		float z = p_coord[ 2 ] - z;

		return sqrt( x * x + y * y + z * z );
	}

	float Particle::compute_distance_squared( Particle * point )
	{
		//return 1.;
		const float * p_coord = point->get_coord();

		float dx = p_coord[ 0 ] - x;
		float dy = p_coord[ 1 ] - y;
		float dz = p_coord[ 2 ] - z;

		return  dx * dx + dy * dy + dz * dz;
		
	}

	void Particle::apply_brownian_mvt( float speed, Vec3f cage_dim ) 
	{ 
		if ( !fix )
		{
			SIM_PART::Vec3f dir;
			dir.x = ( ( getRandomFloat() * 2.f ) - 1.f ) * speed;
			dir.y = ( ( getRandomFloat() * 2.f ) - 1.f ) * speed;
			dir.z = ( ( getRandomFloat() * 2.f ) - 1.f ) * speed;

			x += dir.x;
			y += dir.y;
			z += dir.z;

			//border management
			if ( x < 0 || x > cage_dim.x ) 
				x -= dir.x * 2.f;
			if ( y < 0 || y > cage_dim.y )
				y -= dir.y * 2.f;
			if ( z < 0 || z > cage_dim.z )
				z -= dir.z * 2.f;

			coord[ 0 ] = x;
			coord[ 1 ] = y;
			coord[ 2 ] = z;
		}
	}
	void Particle::compute_attract_by_double_radius( const float				  rayon,
													 const std::vector<Particle *> & pointList,
												  std::vector<int> &		   traveled_point,
												  int						   iteration,
												  int						   refresh_frame )
	{
		point_attract.erase( point_attract.begin(), ( point_attract.begin() + taille_attract - 1 ) );
		Particle * p;
		for ( int i = 0; i < possible_futur_attract.size(); i++ )
		{
			if ( possible_futur_attract[ i ] > id )
			{
				p = pointList[ possible_futur_attract[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->point_attract.emplace_back( p->id );
					p->addAttract( id );
				}
			}
		}
		taille_attract = point_attract.size();
	}
	//===============test==================


	void Particle::compute_diffusion_limited_aggregation( const float				   rayon, 
												  const std::vector<Particle *> & pointList,
												  std::vector<int>	   &traveled_point,
												  int				   iteration,
												  int				   refresh_frame )
	{
		if ( !fix )
		{
			if (taille_attract != 0 )
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
						//std::cout << "taille attract " << p->getPointAttract()->size() << std::endl << std::flush;
						p->addAttract( id );
						if ( p->get_fix()==true )
						{
							std::cout << "FIXED !: " << this->id << std::endl;
							fix = true;
						}
						
							
					}
				}
			}
			taille_attract = point_attract.size();
		}
	}


	void Particle::compute_attract_by_flooding( const float					 rayon, 
												const std::vector<Particle *> & pointList,
												 std::vector<int>	  &traveled_point,
												 int				  iteration,
												 int				  refresh_frame,
												 int				  degre_voisinage)
	{
		point_attract.clear();
		for ( int i = 0; i < traveled_point.size(); i++ )
		{
			traveled_point[ i ] = -1;
		}
		traveled_point[ this->id ] = this->id;

		for ( int i = 0; i < neighbours.size(); i++ )
		{
			traveled_point[ neighbours[ i ]  ] = id;
		}
			
		std::vector<int> n = neighbours;
		std::vector<int> n2;
		Particle *		 p;
		while (degre_voisinage != 0) 
		{
			for (int i = 0; i < n.size(); i++) 
			{
				p  = pointList[ n[ i ] ];
				if ( this->is_attract( p, rayon ) )
				{
					this->point_attract.emplace_back( p->get_id() );
				}
				
				std::vector<int> neighbourg_i = (*pointList[ n[ i ] ]->get_neighbours());
				for ( int j = 0; j < neighbourg_i.size(); j++ ) 
				{
					if ( traveled_point[ neighbourg_i[ j ] ] != id )
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