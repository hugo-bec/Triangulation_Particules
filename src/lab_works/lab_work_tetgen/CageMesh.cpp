#include "CageMesh.hpp"


namespace SIM_PART{

	CageMesh CageMesh::_createCage()
	{
		CageMesh cage		   = CageMesh();
		cage._vertices	   = {	Vec3f( 1, 1, 1 ),  Vec3f( 1, 1, 0 ),  Vec3f( 1, 0, 0 ), Vec3f( 1, 0, 1 ),
								Vec3f( 0, 1, 1 ), Vec3f( 0, 1, 0 ), Vec3f( 0, 0, 0 ), Vec3f( 0, 0, 1 ) };

		cage._segments = {	0, 1, 1, 2, 2, 3, 3, 0, 
							4, 5, 5, 6, 6, 7, 7, 4, 
							0, 4, 1, 5, 2, 6, 3, 7 };
		return cage;
	}

	void CageMesh::_initBuffersCage()
	{ 
		//VBO Points
		glCreateBuffers( 1, &_vboPoints );
		glNamedBufferData( _vboPoints,
						   _vertices.size() * sizeof( Vec3f ),
						   _vertices.data(),
						   GL_STATIC_DRAW );

		//EBO segments
		glCreateBuffers( 1, &_ebo );
		glNamedBufferData( _ebo,
						   _segments.size() * sizeof( unsigned int ),
						   _segments.data(),
						   GL_STATIC_DRAW );

		//VAO
		GLuint indexVBO_points = 0;
		glCreateVertexArrays( 1, &_vao );

		// liaison VAO avec VBO Points
		glEnableVertexArrayAttrib( _vao, indexVBO_points );
		glVertexArrayAttribFormat( _vao,
								   indexVBO_points,
								   3 /*car Vec3f*/,
								   GL_FLOAT /*car Vec3f*/,
								   GL_FALSE, /*non normalis�*/
								   0 /*aucune sparation entre les elements*/ );


		glVertexArrayVertexBuffer( _vao, indexVBO_points, _vboPoints, 0 /*dbute 0*/, sizeof( Vec3f ) );
		//connexion avec le shader (layout(location = 0))
		glVertexArrayAttribBinding( _vao, 0, indexVBO_points ); 

		// liaison VAO avec l'EBO
		glVertexArrayElementBuffer( _vao, _ebo );
	}
}
