#include "triangle_mesh.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

namespace SIM_PART
{
	TriangleMesh::TriangleMesh( const std::string &				  p_name,
								const std::vector<Vertex>	  p_vertices,
								const std::vector<unsigned int> p_indices,
								const Material &				  p_material ) :
		_name( p_name ),
		_vertices( p_vertices ), _indices( p_indices ), _material( p_material )
	{
		_vertices.shrink_to_fit();
		_indices.shrink_to_fit();
		//_vertices[ 0 ]._color = Vec3f( 1, 0, 0 );
		_setupGL();
	}

	void TriangleMesh::render( const GLuint p_glProgram, Mat4f transformation ) const
	{
		glBindVertexArray( _vao ); /*bind VAO avec le programme*/

		//copies of variables for the vertex shader
		glProgramUniformMatrix4fv( p_glProgram, glGetUniformLocation( p_glProgram, "uModelMatrix" ), 1, GL_FALSE, glm::value_ptr( transformation ) );
		glProgramUniform3fv( p_glProgram, glGetUniformLocation( p_glProgram, "ambientColor" ), 1, glm::value_ptr(_material._ambient) );
		glProgramUniform3fv( p_glProgram, glGetUniformLocation( p_glProgram, "difuseColor" ), 1, glm::value_ptr(_material._diffuse) );
		glProgramUniform3fv( p_glProgram, glGetUniformLocation( p_glProgram, "sourceLight" ), 1, glm::value_ptr(Vec3f(0.f, 15.f, 0.f)) );

		//launching OpenGL graphics pipeline
		glDrawElements( GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0 );

		glBindVertexArray( 0 ); /*debind VAO*/
	}

	void TriangleMesh::update_color( Vec3f color )
	{
		//if ( color.x != 0 || color.y != 0 || color.z != 0 ) std::cout << "COLOR != NOIR" << std::endl;
		for ( int i = 0; i < _vertices.size(); i++ )
			_vertices[ i ]._color = color;


		glNamedBufferData( _vbo, _vertices.size() * sizeof( Vertex ), _vertices.data(), GL_DYNAMIC_DRAW );
		//Vec3f ctmp = _vertices[ 0 ]._color;
		//if ( ctmp.x != 0 || ctmp.y != 0 || ctmp.z != 0 )
			//std::cout << "COLOR: " << ctmp.x << ", " << ctmp.y << ", " << ctmp.z << std::endl;
	}
	     
	void TriangleMesh::cleanGL()
	{
		glDisableVertexArrayAttrib( _vao, 0 );
		glDisableVertexArrayAttrib( _vao, 1 );
		glDisableVertexArrayAttrib( _vao, 2 );
		glDeleteVertexArrays( 1, &_vao );
		glDeleteBuffers( 1, &_vbo );
		glDeleteBuffers( 1, &_ebo );
	}

	void TriangleMesh::_setupGL()
	{		
		// creation vbo
		glCreateBuffers( 1, &_vbo );
		glNamedBufferData( _vbo, _vertices.size() * sizeof( Vertex ), _vertices.data(), GL_DYNAMIC_DRAW );
		
		// creation ebo
		glCreateBuffers( 1, &_ebo );
		glNamedBufferData( _ebo, _indices.size() * sizeof( unsigned int ), _indices.data(), GL_DYNAMIC_DRAW );

		// creation vao
		glCreateVertexArrays( 1, &_vao );
		GLuint index_pos	= 0;
		GLuint index_color	= 1;
		GLuint index_normal = 2;
		glVertexArrayVertexBuffer( _vao, 0, _vbo, 0, sizeof( Vertex ) );

		// positions
		glEnableVertexArrayAttrib( _vao, index_pos );
		glVertexArrayAttribFormat( _vao,
								   index_pos,
								   3 /*car Vec(3)f*/,
								   GL_FLOAT /*car Vec3(f)*/,
								   GL_FALSE, /*non normalisé*/
								   offsetof( Vertex, _position ) );
		glVertexArrayAttribBinding( _vao, index_pos, 0 );

		// colors
		glEnableVertexArrayAttrib( _vao, index_color );
		glVertexArrayAttribFormat( _vao,
								   index_color,
								   3 /*car Vec(3)f*/,
								   GL_FLOAT /*car Vec3(f)*/,
								   GL_FALSE, /*non normalisé*/
								   offsetof( Vertex, _color ) );
		glVertexArrayAttribBinding( _vao, index_color, 0 );

		// normale
		glEnableVertexArrayAttrib( _vao, index_normal );
		glVertexArrayAttribFormat( _vao,
								   index_normal,
								   3 /*car Vec(3)f*/,
								   GL_FLOAT /*car Vec3(f)*/,
								   GL_FALSE, /*non normalisé*/
								   offsetof( Vertex, _normal ) );
		glVertexArrayAttribBinding( _vao, index_normal, 0 );

		// liaison vao avec ebo
		glVertexArrayElementBuffer( _vao, _ebo );
	}
} // namespace M3D_ISICG
