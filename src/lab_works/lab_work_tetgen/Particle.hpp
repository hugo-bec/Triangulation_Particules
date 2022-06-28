#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
#include <cstdio>
#include <set>
#include "define.hpp"

namespace SIM_PART
{
	class Tetrahedron;

	class Particle
	{

	  public:
		Particle() {};

		Particle( int _id, float _x, float _y, float _z ) : id( _id ), x( _x ), y( _y ), z( _z ) 
		{
			coord[ 0 ] = x;
			coord[ 1 ] = y;
			coord[ 2 ] = z;
		};

		~Particle() {};

		// static bool comparatorSet(Point a, Point b){ return a.samePoints(&b); }

		const float* get_coord() const;
		int				   get_id();
		const std::vector<int>*   get_tetrahedron();
		const std::vector<int>*   get_point_attract();
		const std::vector<int>*   get_neighbours();
		void			   addNeighbour( int i ) { neighbours.emplace_back( i ); }
		inline void		   addPossibleAttract( int i ) { possible_futur_attract.emplace_back( i ); }
		void			   addAttract( int i ) { point_attract.push_back( i ); }
		void			   tri_voisin();
		
		inline void printCoord() { printf( "(x: %lf, y: %lf, z: %lf\n)", x, y, z ); };

		bool is_attract( Particle * p, float attract_distance );

		void add_point( Particle * p );

		void compute_neighbours( std::vector<Tetrahedron *> tetraList );

		void compute_neighbours_v2( std::vector<Tetrahedron *> tetraList );

		bool is_same( Particle * p );

		void compute_point_attract( float r, std::vector<Particle *> PointList );

		void add_tetrahedron( Tetrahedron * t );

		Particle * find_point( std::vector<Particle *> PointList, int id );

		Tetrahedron * find_tetra( std::vector<Tetrahedron *> PointList, int id );

		

		void compute_point_attract_v4( float				 r,
									const std::vector<Particle *> &PointList,
									std::vector<int>	 &traveled_point,
									int					 refresh_frame );


		//=============Point Attract Version brute============

		void compute_point_attract_brut( float r, std::vector<Particle *> PointList );

		//=============Trouver point attract sans refaire les tétrahèdres============
		
		void compute_attract_by_double_radius( const float					rayon, 
											   const std::vector<Particle *> &pointList,
											   std::vector<int>		&traveled_point,
											   int					iteration,
											   int					refresh_frame );


		void compute_attract_by_flooding(	const float				   rayon, 
												const std::vector<Particle *> &pointList,
												std::vector<int>	  &traveled_point,
												int				  iteration,
												int				  refresh_frame, 
												int degre_voisinage );
		
		inline float compute_distance( Particle * point );

		inline float compute_distance_squared( Particle * point );

		bool get_fix() { return fix; }

		void apply_brownian_mvt( float speed, Vec3f dimCage );

		void set_coord( const float px, const float py, const float pz );

		void set_fix( bool b ) { fix = b; }

		void compute_diffusion_limited_aggregation( const float				  rayon,
												 const std::vector<Particle *> & pointList,
												 std::vector<int> &			  traveled_point,
												 int						  iteration,
												 int						  refresh_frame );

		
		
	  private:
		int	  id;
		float x;
		float y;
		float z;
		float coord[ 3 ];
		float speed = 0.01f;
		//float rayon = 2.f;

		std::vector<int> tetra;
		std::vector<int> point_attract;
		std::vector<int> neighbours;
		std::vector<int> possible_futur_attract;
		int				 taille_attract;

		//Diffusion Limited Aggregation
		bool fix = false;
		Vec3f color;
		
	};
} // namespace tetrasearch

#endif