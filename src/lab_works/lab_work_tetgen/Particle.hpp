#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
#include <cstdio>
#include "define.hpp"
#include "common/models/triangle_mesh_model.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace SIM_PART
{
	class Tetrahedron;

	class Particle
	{

	  public:

		Particle( int p_id, float p_x, float p_y, float p_z, TriangleMeshModel & origin_model, float size ) : _id( p_id ) 
		{
			_coord[ 0 ] = p_x;
			_coord[ 1 ] = p_y;
			_coord[ 2 ] = p_z;
			_model.load_with_model( "p" + std::to_string(p_id), origin_model );
			_model._transformation = glm::translate( _model._transformation, Vec3f(p_x, p_y, p_z) );
			_model._transformation = glm::scale( _model._transformation, Vec3f(size) );


		}

		~Particle() {};

		const float *            get_coord() const { return _coord; }
		int                      get_id() const { return _id; }
		const std::vector<int> * get_tetrahedron() const { return &_tetras; }
		const std::vector<int> * get_point_attract() const { return &_particules_attract; }
		const std::vector<int> * get_neighbours() const { return &_neighbours; }
		bool                     is_fix() const { return _fix; }

		void add_point( Particle * p ) { _particules_attract.emplace_back( p->get_id() ); }
		void add_tetrahedron( Tetrahedron * t );
		void add_neighbour( int i ) { _neighbours.emplace_back( i ); }
		void add_possible_attract( int i ) { _possible_futur_attract.emplace_back( i ); }
		void add_attract( int i ) { _particules_attract.emplace_back( i ); }

		void clear_tetrahedron() { _tetras.clear(); }

		void set_fix( const bool b ) { _fix = b; }
		void set_coord( float px, float py, float pz );
		void set_coord_mesh( float px, float py, float pz );

		void printCoord() const { printf( "(x: %lf, y: %lf, z: %lf\n)", _coord[ 0 ], _coord[ 1 ], _coord[ 2 ] ); };
		void tri_voisin();
		
		void render( GLuint program ) { _model.render( program ); }

		bool is_attract( Particle * p, float attract_distance ) const;

		void compute_neighbours( std::vector<Tetrahedron *> tetra_list );

		void compute_neighbours_v2( std::vector<Tetrahedron *> tetra_list );

		bool is_same( Particle * p ) const;

		void compute_point_attract( float r, std::vector<Particle *> point_list );


		void compute_point_attract_v4(	float				 r,
										const std::vector<Particle *> &point_list,
									   std::vector<int> &			   traveled_point,
									   int							   refresh_mesh );

		void compute_point_attract_parallelisable(	float						   r, const std::vector<Particle *> & point_list, int refresh_mesh
												   /* std::vector<int>		   traveled_point*/ );

		void compute_point_attract_parallelisable_v2( float							  r,
													  const std::vector<Particle *> & pointList,
													  std::vector<int> &			  traveled_point,
													  int							  refresh_mesh );


		//=============Point Attract Version brute============

		void compute_point_attract_brut( float r, std::vector<Particle *> point_list );

		//=============Trouver point attract sans refaire les t�trah�dres============
		
		void compute_attract_by_double_radius( float                           rayon, 
		                                       const std::vector<Particle *> & point_list,
		                                       std::vector<int>		&          traveled_point,
		                                       int                             iteration,
		                                       int                             refresh_frame );

		void compute_attract_by_double_radius_parallelisable(	const float						rayon,
																const std::vector<Particle *> & point_list,
																std::vector<int> &				traveled_point,
																int								iteration,
																int								refresh_frame );


		void compute_attract_by_flooding( float                           rayon, 
		                                  const std::vector<Particle *> & pointList,
		                                  std::vector<int>	             traveled_point,
		                                  int                             iteration,
		                                  int                             refresh_frame, 
		                                  int                             degre_voisinage );
		
		inline float compute_distance( Particle * point ) const;

		inline float compute_distance_squared( Particle * point ) const;

		void apply_brownian_mvt( float speed, Vec3f dimCage );

		

		

		void compute_diffusion_limited_aggregation( float                           rayon,
		                                            const std::vector<Particle *> & pointList,
		                                            std::vector<int> &              traveled_point,
		                                            int                             iteration,
		                                            int                             refresh_frame,
													int								nb_non_fix );

		void compute_diffusion_limited_aggregation_V2( const float					 rayon,
																 const std::vector<Particle *> & pointList,
																 std::vector<int> &				 traveled_point,
																 int							 iteration,
																   int							   refresh_frame,
																   int							   nb_non_fix );
		
	  private:
		int	  _id;
		float _coord[ 3 ];
		float _speed = 0.01f;
		Vec3f _color;
		TriangleMeshModel _model;

		std::vector<int> _tetras;
		std::vector<int> _particules_attract;
		std::vector<int> _neighbours;
		std::vector<int> _possible_futur_attract;
		int				 _taille_attract;

		//Diffusion Limited Aggregation
		bool	_fix = false;
		
	};
} // namespace tetrasearch

#endif