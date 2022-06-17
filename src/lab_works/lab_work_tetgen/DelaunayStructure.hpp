#pragma once
#include "define.hpp"
#include "gl3w/GL/gl3w.h"
#include <vector>

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




		// ================ Geometric data.
		std::vector<Vec3f>		  _positions;
		std::vector<Vec3f>		  _colors;
		std::vector<unsigned int> _indices;
		// std::vector<gluSphere> ;
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
