#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
#include <cstdio>
#include "define.hpp"
#include "parameters.hpp"
#include "common/models/triangle_mesh_model.hpp"
#include "glm/gtc/type_ptr.hpp"


namespace SIM_PART
{
	class Tetrahedron;

	class Particle
	{

	  public:

		Particle( int p_id, float p_x, float p_y, float p_z, TriangleMeshModel & p_model, float p_size, float p_speed ) :
			_id( p_id ),
			_speed( p_speed ), _model( p_model )
		{
			_coord[ 0 ] = p_x;
			_coord[ 1 ] = p_y;
			_coord[ 2 ] = p_z;
			//_model.load_with_model( "p" + std::to_string(p_id), p_origin_model );
			//_model.load( "p" + std::to_string( p_id ), "data/model/icosphere2.obj" );
			
			_model._transformation = glm::translate( _model._transformation, Vec3f( p_x, p_y, p_z ) );
			_model._transformation = glm::scale( _model._transformation, Vec3f( p_size ) );
		}

		~Particle() {};

		void create_model();

		const float *            get_coord() const { return _coord; }
		int                      get_id() const { return _id; }
		const std::vector<int> * get_tetrahedron() const { return &_tetras; }
		const std::vector<int> * get_point_attract() const { return &_particules_attract; }
		const std::vector<int> * get_neighbours() const { return &_neighbours; }
		const Vec3f 			 get_color() const { return _color; }
		bool                     is_fix() const { return _fix; }

		void add_point( Particle * p ) { _particules_attract.emplace_back( p->get_id() ); }
		void add_tetrahedron( Tetrahedron * t );
		void add_neighbour( int i ) { _neighbours.emplace_back( i ); }
		void add_possible_attract( int i ) { _possible_futur_attract.emplace_back( i ); }
		void add_attract( int i ) { _particules_attract.emplace_back( i ); }

		void clear_tetrahedron() { _tetras.clear(); }

		void set_fix( const int p_nb_frame ) {
			if (!_fix) {_fix = true; _nb_frame_free = p_nb_frame;}
		}
		void set_attract( const bool b ) { _attract = b; }
		void set_speed( const float s ) { _speed = s; }
		void set_coord( float px, float py, float pz );
		void set_coord_mesh( float px, float py, float pz );
		void set_color( Vec3f color ) { _color = color; }

		void printCoord() const { printf( "(x: %lf, y: %lf, z: %lf\n)", _coord[ 0 ], _coord[ 1 ], _coord[ 2 ] ); };
		void tri_voisin();

		void compute_coloration( int mode_type );
		void update_mesh() { _model.setColor( _color ); }
		void render( GLuint program ) { _model.render( program ); }

		bool is_attract( Particle * p, float attract_distance ) const;


		void compute_point_attract_parallelisable_brut( float							r,
														const std::vector<Particle *> & point_list,
														int								refresh_mesh );

		void compute_point_attract_parallelisable_without_double_radius(  float							  r,
																		  const std::vector<Particle *> & pointList,
																		  std::vector<int> &			  traveled_point );

		void compute_point_attract_parallelisable_double_radius( float							 r,
																 const std::vector<Particle *> & pointList,
																 std::vector<int> &				 traveled_point,
																 int							 refresh_mesh );
		//=============Trouver points attracts sans refaire les tetrahedres============
		

		void compute_attract_by_double_radius(	const float						rayon,
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

		
		

		void compute_diffusion_limited_aggregation( float							rayon,
													const std::vector<Particle *> & pointList,
													int								nb_non_fix,
													int								time_frame );


		
		inline float compute_distance( Particle * point ) const;

		inline float compute_distance_squared( Particle * point ) const;

		void apply_brownian_mvt( float speed, Vec3f dimCage );

		
		

		
	  private:
		int	  _id;
		float _coord[ 3 ];
		float _speed = SPEED_PARTICULES;
		Vec3f			  _color = Vec3f(0);
		TriangleMeshModel _model;

		std::vector<int> _tetras;
		std::vector<int> _particules_attract;
		std::vector<int> _neighbours;
		std::vector<int> _possible_futur_attract;

		bool	_fix = false;
		bool	_attract = false;
		int		_nb_frame_free = 0;
		
	};
} // namespace tetrasearch

#endif