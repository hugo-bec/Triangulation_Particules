#ifndef __TRIANGLE_MESH_HPP__
#define __TRIANGLE_MESH_HPP__

#include "GL/gl3w.h"
#include "define.hpp"
#include <iostream>
#include <vector>

namespace SIM_PART
{
	struct Vertex
	{
		Vec3f _position;
		Vec3f _normal;
	};

	struct Material
	{
		Vec3f _ambient	 = VEC3F_ZERO;
		Vec3f _diffuse	 = VEC3F_ZERO;
		Vec3f _specular	 = VEC3F_ZERO;
		float _shininess = 0.f;
	};

	class TriangleMesh
	{
	  public:
		TriangleMesh() = delete;
		TriangleMesh( const std::string &				p_name,
					  const std::vector<Vertex> &		p_vertices,
					  const std::vector<unsigned int> & p_indices,
					  const Material &					p_material );

		~TriangleMesh() = default;

		void render( const GLuint p_glProgram, Mat4f transformation ) const;

		void cleanGL();

	  private:
		void _setupGL();

	  public:
		std::string _name = "Unknown";

		// ================ Geometric data.
		std::vector<Vertex>		  _vertices;
		std::vector<unsigned int> _indices;

		// ================ Material data.
		Material _material;

		// ================ GL data.
		GLuint _vao = GL_INVALID_INDEX; // Vertex Array Object
		GLuint _vbo = GL_INVALID_INDEX; // Vertex Buffer Object
		GLuint _ebo = GL_INVALID_INDEX; // Element Buffer Object
	};
} // namespace M3D_ISICG

#endif // __TRIANGLE_MESH_HPP__
