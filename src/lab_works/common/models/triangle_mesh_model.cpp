#include "triangle_mesh_model.hpp"
#include "lab_work_tetgen/Particle.hpp"
#include "utils/image.hpp"
#include <iostream>
#include <assimp/postprocess.h>

namespace SIM_PART
{
	void TriangleMeshModel::load( const std::string & p_name, const FilePath & p_filePath )
	{
		_name = p_name;
		std::cout << "Loading model " << _name << " from: " << p_filePath << std::endl;
		_dirPath = p_filePath.dirPath();

		Assimp::Importer importer;

		// Importer options
		// Cf. http://assimp.sourceforge.net/lib_html/postprocess_8h.html.
		const unsigned int flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs;

		// Read scene :
		// - Triangulates meshes
		// - Computes vertex normals
		const aiScene * const scene = importer.ReadFile( p_filePath, flags );

		if ( scene == nullptr )
			throw std::runtime_error( "Fail to load file \" " + p_filePath.str() + "\": " + importer.GetErrorString() );

		_meshes.reserve( scene->mNumMeshes );
		for ( unsigned int i = 0; i < scene->mNumMeshes; ++i )
		{
			_loadMesh( scene->mMeshes[ i ], scene );
		}
		_meshes.shrink_to_fit();

		std::cout << "Done! "						//
				  << _meshes.size() << " meshes, "	//
				  << _nbTriangles << " triangles, " //
				  << _nbVertices << " vertices" << std::endl;
	}

	void TriangleMeshModel::load_multiple_model(	const std::string & p_name, 
													const FilePath & p_filePath,
													std::vector<TriangleMeshModel* > & tmm_container,
													const int nb_copy )
	{
		_name = p_name;
		std::cout << "Loading model " << _name << " from: " << p_filePath << std::endl;
		_dirPath = p_filePath.dirPath();

		Assimp::Importer importer;

		// Importer options
		// Cf. http://assimp.sourceforge.net/lib_html/postprocess_8h.html.
		const unsigned int flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs;

		// Read scene :
		// - Triangulates meshes
		// - Computes vertex normals
		const aiScene * const scene = importer.ReadFile( p_filePath, flags );

		if ( scene == nullptr )
			throw std::runtime_error( "Fail to load file \" " + p_filePath.str() + "\": " + importer.GetErrorString() );
		_meshes.reserve( scene->mNumMeshes );
		
		const aiMesh *	  p_mesh   = scene->mMeshes[ 0 ];
		const std::string meshName = _name + "_" + std::string( p_mesh->mName.C_Str() );
		if ( VERBOSE ) std::cout << "-- Loading mesh: " << meshName << std::endl;

		// Load vertex attributes.
		std::vector<Vertex> vertices;
		vertices.resize( p_mesh->mNumVertices );
		for ( unsigned int v = 0; v < p_mesh->mNumVertices; ++v )
		{
			Vertex & vertex = vertices[ v ];
			// Position.
			vertex._position.x = p_mesh->mVertices[ v ].x;
			vertex._position.y = p_mesh->mVertices[ v ].y;
			vertex._position.z = p_mesh->mVertices[ v ].z;
			// Normal.
			vertex._normal.x = p_mesh->mNormals[ v ].x;
			vertex._normal.y = p_mesh->mNormals[ v ].y;
			vertex._normal.z = p_mesh->mNormals[ v ].z;
		}

		// Load indices.
		std::vector<unsigned int> indices;
		indices.resize( p_mesh->mNumFaces * 3 ); // Triangulated.
		for ( unsigned int f = 0; f < p_mesh->mNumFaces; ++f )
		{
			const aiFace &	   face = p_mesh->mFaces[ f ];
			const unsigned int f3	= f * 3;
			indices[ f3 ]			= face.mIndices[ 0 ];
			indices[ f3 + 1 ]		= face.mIndices[ 1 ];
			indices[ f3 + 2 ]		= face.mIndices[ 2 ];
		}

		Material			material;
		TriangleMeshModel * tmm;
		for ( int i = 0; i < nb_copy; i++ )
		{
			tmm = new TriangleMeshModel();
			tmm->_nbTriangles = p_mesh->mNumFaces;
			tmm->_nbVertices = p_mesh->mNumVertices;
			TriangleMesh cheval = TriangleMesh( "caca", vertices, indices, material );

			tmm->_meshes.emplace_back( cheval );
			tmm_container.emplace_back(tmm);
		}

	}


	void TriangleMeshModel::render( const GLuint p_glProgram ) const
	{
		for ( size_t i = 0; i < _meshes.size(); i++ )
		{
			_meshes[ i ].render( p_glProgram, _transformation );
		}
	}

	void TriangleMeshModel::cleanGL()
	{
		for ( size_t i = 0; i < _meshes.size(); i++ )
		{
			_meshes[ i ].cleanGL();
		}
	}

	void TriangleMeshModel::_loadMesh( const aiMesh * const p_mesh, const aiScene * const p_scene )
	{
		const std::string meshName = _name + "_" + std::string( p_mesh->mName.C_Str() );
		if ( VERBOSE )
			std::cout << "-- Loading mesh: " << meshName << std::endl;

		// Load vertex attributes.
		std::vector<Vertex> vertices;
		vertices.resize( p_mesh->mNumVertices );
		for ( unsigned int v = 0; v < p_mesh->mNumVertices; ++v )
		{
			Vertex & vertex = vertices[ v ];
			// Position.
			vertex._position.x = p_mesh->mVertices[ v ].x;
			vertex._position.y = p_mesh->mVertices[ v ].y;
			vertex._position.z = p_mesh->mVertices[ v ].z;
			// Normal.
			vertex._normal.x = p_mesh->mNormals[ v ].x;
			vertex._normal.y = p_mesh->mNormals[ v ].y;
			vertex._normal.z = p_mesh->mNormals[ v ].z;
		}

		// Load indices.
		std::vector<unsigned int> indices;
		indices.resize( p_mesh->mNumFaces * 3 ); // Triangulated.
		for ( unsigned int f = 0; f < p_mesh->mNumFaces; ++f )
		{
			const aiFace &	   face = p_mesh->mFaces[ f ];
			const unsigned int f3	= f * 3;
			indices[ f3 ]			= face.mIndices[ 0 ];
			indices[ f3 + 1 ]		= face.mIndices[ 1 ];
			indices[ f3 + 2 ]		= face.mIndices[ 2 ];
		}

		// Load materials.
		Material material;
		_nbTriangles += p_mesh->mNumFaces;
		_nbVertices += p_mesh->mNumVertices;

		_meshes.emplace_back( TriangleMesh( meshName, vertices, indices, material ) );

		if ( VERBOSE )
		{
			std::cout << "-- Done! "						  //
					  << indices.size() / 3 << " triangles, " //
					  << vertices.size() << " vertices." << std::endl;
		}
	}

} // namespace M3D_ISICG
