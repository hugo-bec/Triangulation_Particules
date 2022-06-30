#pragma once
#include <iostream>
#include <vector>
#include "gl3w/GL/gl3w.h"

#include "define.hpp"
#include "utils/Chrono.hpp"
#include "tetgen.h"
#include "Particle.hpp"
#include "Tetrahedron.hpp"


namespace SIM_PART
{
	class DelaunayStructure
	{
	  public:

		~DelaunayStructure()
		{
			if ( _vao != GL_INVALID_INDEX )
			{
				glDisableVertexArrayAttrib( _vao, 0 );
				glDisableVertexArrayAttrib( _vao, 1 );
				glDeleteVertexArrays( 1, &_vao );
			}
			if ( _vboPoints != GL_INVALID_INDEX )
				glDeleteBuffers( 1, &_vboPoints );
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
		inline void set_edges_mode( bool b ) { _edges_mode = b; }
		inline void set_play_mode( bool b ) { _play_mode = b; }
		inline void set_type_mode()
		{
			if ( _mode_type == 0 )
				_mode_type = 1;
			else
				_mode_type = 0;
		}

		inline void set_active_particle( int i ) { _active_particle = i < 0 ? _nbparticules - (i*(-1))%_nbparticules : i % _nbparticules; }

		/* --- INITIALIZATION FUNCTIONS --- */ 
		void init_particules( const std::vector<Particle *> & p_particules );
		void init_structure();
		void init_buffers();
		void init_all( const std::vector<Particle *> & p_particules );

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
		void render( GLuint _program, GLuint _uModelMatrixLoc );
		void coloration();


		/* -----------------------------------------------------------------------
		 * ----------------------------- ATTRIBUTES ------------------------------
		 * ----------------------------------------------------------------------- */

		// ================ Geometric data.
		Vec3f _dimCage = Vec3f(10);
		int	  _nbparticules;
		float _rayon_attract = 0.1f;
		int	  nb_non_fix=0;

		std::vector<Particle *>		_list_points;
		std::vector<Tetrahedron *>	_list_tetras;
		tetgenio					_tetgen_mesh;

		std::vector<Vec3f>		  _positions;
		std::vector<Vec3f>		  _colors;
		std::vector<unsigned int> _indices;
		std::vector<int>		  _traveled_point;
		int						  _refresh_frame = 100;

		Mat4f  _transformation = MAT4F_ID;

		// ================ Rendering & Utils data.
		Chrono	_chrono;
		bool	_verbose		 = false;
		bool	_draw_all_edges	 = false;
		bool	_edges_mode		 = false;
		bool	_play_mode		 = false;
		int		_mode_type		 = 1;
		int		_active_particle = 0;
		int		_iteration		 = 1;



		// ================ GL data.
		GLuint _vao = GL_INVALID_INDEX; // Vertex Array Object
		GLuint _ebo = GL_INVALID_INDEX; // Element Buffer Object

		// Vertex Buffer Objects.
		GLuint _vboPoints = GL_INVALID_INDEX;
		GLuint _vboColors = GL_INVALID_INDEX;
		// ================
	};

} // namespace SIM_PART
