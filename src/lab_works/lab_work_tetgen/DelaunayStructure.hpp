#pragma once
#include <iostream>
#include <vector>
#include "gl3w/GL/gl3w.h"

#include "define.hpp"
#include "utils/Chrono.hpp"
#include "tetgen.h"
#include "Particle.hpp"
#include "Tetrahedron.hpp"
#include "parameters.hpp"


namespace SIM_PART
{
	class DelaunayStructure
	{
	  public:

		~DelaunayStructure()
		{
			if ( _vaoParticules != GL_INVALID_INDEX )
			{
				glDisableVertexArrayAttrib( _vaoParticules, 0 );
				glDisableVertexArrayAttrib( _vaoParticules, 1 );
				glDeleteVertexArrays( 1, &_vaoParticules );
			}
			if ( _vaoMesh != GL_INVALID_INDEX )
			{
				glDisableVertexArrayAttrib( _vaoMesh, 0 );
				glDisableVertexArrayAttrib( _vaoMesh, 1 );
				glDeleteVertexArrays( 1, &_vaoMesh );
			}
			if ( _vboPositions != GL_INVALID_INDEX )
				glDeleteBuffers( 1, &_vboPositions );
			if ( _vboColors != GL_INVALID_INDEX )
				glDeleteBuffers( 1, &_vboColors );
			if ( _vboVertices != GL_INVALID_INDEX )
				glDeleteBuffers( 1, &_vboVertices );
		}

		/* -----------------------------------------------------------------------
		 * ----------------------------- FUNCTIONS -------------------------------
		 * ----------------------------------------------------------------------- */

		/* --- SETTER FUNCTIONS --- */ 
		inline void set_verbose( bool b )
		{
			_verbose = b;
			_chrono.set_verbose( b );
		}

		inline void set_draw_all_edges( bool b ) { _draw_all_edges = b; }
		inline void set_point_mode( bool b ) { _point_mode = b; }
		inline void set_edges_mode( bool b ) { _edges_mode = b; }
		inline void set_play_mode( bool b ) { _play_mode = b; }
		inline void set_type_mode()
		{
			if ( _mode_type == 0 )
				_mode_type = 1;
			else
				_mode_type = 0;
		}

		inline void set_active_particle( int i )
		{
			_active_particle = i < 0 ? NB_PARTICULES - ( i * ( -1 ) ) % NB_PARTICULES : i % NB_PARTICULES;
		}

		/* --- INITIALIZATION FUNCTIONS --- */ 
		void init_particules( const std::vector<Particle *> & p_particules, int p_refresh_rate );
		void init_structure();
		void init_mesh();
		void init_buffers();
		void init_all( GLuint program, const std::vector<Particle *> & p_particules, int p_refresh_rate );

		/* --- UPDATE FUNCTIONS --- */ 
		void tetrahedralize_particules( char * tetgen_parameters );
		void update_position_particules();
		void update_structure();
		void update_tetras();
		void update_rendering();
		void update_buffers();
		void update_all();

		/* --- COMPUTE & RENDER FUNCTIONS --- */ 
		void compute_attract_points();
		void render( GLuint _program );

		void coloration();


		/* -----------------------------------------------------------------------
		 * ----------------------------- ATTRIBUTES ------------------------------
		 * ----------------------------------------------------------------------- */

		// ================ Geometric data.
		//Vec3f _dimCage;
		//int	  _nbparticules;
		//float _rayon_attract;
		int	  nb_non_fix	 = NB_PARTICULES - NB_INIT_FIXED_POINTS;
		std::vector<int> _traveled_point;
		int				 _refresh_frame;

		std::vector<Particle *>		_list_points;
		std::vector<Tetrahedron *>	_list_tetras;
		tetgenio					_tetgen_mesh;

		std::vector<Vec3f>		  _positions;
		std::vector<Vec3f>		  _colors;
		std::vector<unsigned int> _indices;
		std::vector<Vertex>		  _mesh_vertices;
		std::vector<unsigned int> _mesh_indices;
		

		Mat4f  _transformation = MAT4F_ID;
		aiMesh * _model;

		// ================ Rendering & Utils data.
		Chrono	_chrono;
		bool	_verbose		 = false;
		bool	_draw_all_edges	 = false;
		bool	_edges_mode		 = false;
		bool	_play_mode		 = false;
		bool	_point_mode		 = false;
		int		_mode_type		 = 1;
		int		_active_particle = 0;
		int		_degre_voisinage = 2;
		int		_iteration		 = 1;



		// ================ GL data.
		GLuint _vaoParticules = GL_INVALID_INDEX; 
		GLuint _eboParticules = GL_INVALID_INDEX;
		GLuint _vboPositions  = GL_INVALID_INDEX;
		GLuint _vboColors	  = GL_INVALID_INDEX;

		GLuint _vaoMesh = GL_INVALID_INDEX;
		GLuint _eboMesh = GL_INVALID_INDEX;
		GLuint _vboVertices = GL_INVALID_INDEX;


		GLint  _uModelMatrixLoc = GL_INVALID_INDEX;
		// ================
	};

} // namespace SIM_PART
