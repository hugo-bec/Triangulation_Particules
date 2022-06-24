#pragma once
#include <iostream>
#include <vector>
#include "gl3w/GL/gl3w.h"

#include "define.hpp"
#include "utils/Chrono.hpp"
#include "tetgen.h"
#include "Point.hpp"
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

		void init_structure();
		void init_particules( unsigned int p_nbparticules );
		void init_buffers();
		void tetrahedralize_particules( char * tetgen_parameters );

		void update_position_particules( float speed );
		void update_structure();
		void update_rendering( bool print_all_edges, int actif_point );
		void update_tetras();
		void update_buffers();

		void compute_neighbours();
		void compute_attract_points();

		void fix_first_points( int nb_points );
		void set_verbose( bool v );
	

		// ================ Geometric data.
		Vec3f _dimCage = Vec3f(10);
		int	  _nbparticules = 10000;
		float rayon_attract = 0.1f;

		std::vector<Point*>		list_points;
		std::vector<Tetrahedron *> list_tetras;
		tetgenio				   _tetgen_mesh;

		std::vector<Vec3f>		  _positions;
		std::vector<Vec3f>		  _colors;
		std::vector<unsigned int> _indices;
		std::vector<int>		  _traveled_point;
		int						  refresh_frame=100;

		Mat4f  _transformation = MAT4F_ID;
		Chrono _chrono;
		bool   verbose = false;
		// ================

		// ================ GL data.
		GLuint _vao = GL_INVALID_INDEX; // Vertex Array Object
		GLuint _ebo = GL_INVALID_INDEX; // Element Buffer Object

		// Vertex Buffer Objects.
		GLuint _vboPoints = GL_INVALID_INDEX;
		GLuint _vboColors = GL_INVALID_INDEX;
		// ================
	};

} // namespace SIM_PART
