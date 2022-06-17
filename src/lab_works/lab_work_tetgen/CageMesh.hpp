#pragma once
#include<vector>
#include "define.hpp"
#include "GL/gl3w.h"

namespace SIM_PART
{
	class CageMesh
	{
	  public:
		/*CageMesh(std::vector<Vec3f> p_vertices, std::vector<unsigned int> p_segments)
			: _vertices
		{}*/

		~CageMesh()
		{
			if ( _vao != GL_INVALID_INDEX )
			{
				glDisableVertexArrayAttrib( _vao, 0 );
				glDisableVertexArrayAttrib( _vao, 1 );
				glDeleteVertexArrays( 1, &_vao );
			}
			if ( _vboPoints != GL_INVALID_INDEX )
				glDeleteBuffers( 1, &_vboPoints );
			if ( _ebo != GL_INVALID_INDEX )
				glDeleteBuffers( 1, &_ebo );
		}

		static CageMesh _createCage();
		void			_initBuffersCage();

		// ================ Geometric data.
		std::vector<Vec3f>		  _vertices;
		std::vector<unsigned int> _segments;
		Mat4f					  _transformation = MAT4F_ID;
		// ================

		// ================ GL data.
		GLuint _vao = GL_INVALID_INDEX; // Vertex Array Object
		GLuint _ebo = GL_INVALID_INDEX; // Element Buffer Object

		// Vertex Buffer Objects.
		GLuint _vboPoints = GL_INVALID_INDEX;
		// ================
	};
} // namespace SIM_PART
