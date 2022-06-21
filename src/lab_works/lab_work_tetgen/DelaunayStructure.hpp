#pragma once
#include "define.hpp"
#include "gl3w/GL/gl3w.h"
#include <vector>
#include "tetgen.h"
#include "Point.hpp"
#include "Tetrahedron.hpp"
#include <iostream>

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

		void _initBuffersParticules();
		void tetrahedralize_particules( tetgenio * in, tetgenio * out );
		void _createParticules();
		void _colorPoint( bool print_all_edges, int actif_point  );
		void init_particules( tetgenio * in );
		void update_particules( tetgenio * out );
		void update_points_tetras( tetgenio * out );
		void compute_neighbours();
		void compute_attract_points();

		// ================ Geometric data.
		Vec3f _dimCage = Vec3f(10);
		int	  _nbparticules = 10000;
		float rayon_attract = 2.f;

		std::vector<Point*>		list_points;
		std::vector<Tetrahedron *> list_tetras;
		tetgenio					tetgenMesh;
		

		std::vector<Vec3f>		  _positions;
		std::vector<Vec3f>		  _colors;
		std::vector<unsigned int> _indices;
		std::vector<int>		  _traveled_point;
		int						  refresh_frame=100;

		Mat4f _transformation = MAT4F_ID;
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
