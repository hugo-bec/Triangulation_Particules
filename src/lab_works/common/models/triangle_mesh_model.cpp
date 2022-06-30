#include "triangle_mesh_model.hpp"
#include "utils/image.hpp"
#include <iostream>

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
		// - Computes tangent space (tangent and bitagent)
		const aiScene * const scene = importer.ReadFile( p_filePath, flags );

		if ( scene == nullptr )
		{
			throw std::runtime_error( "Fail to load file \" " + p_filePath.str() + "\": " + importer.GetErrorString() );
		}

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

	void TriangleMeshModel::load_with_model( const std::string & p_name, const TriangleMeshModel & origin_model ) 
	{
		_name = p_name;
		_meshes = origin_model._meshes;
		_loadedTextures = origin_model._loadedTextures;
		_nbTriangles	= origin_model._nbTriangles;
		_nbVertices		= origin_model._nbVertices;
		_dirPath		= origin_model._dirPath;
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
		for ( size_t i = 0; i < _loadedTextures.size(); i++ )
		{
			glDeleteTextures( 1, &( _loadedTextures[ i ]._id ) );
		}
		_loadedTextures.clear();
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
			// Texture coordinates.
			if ( p_mesh->HasTextureCoords( 0 ) )
			{
				vertex._texCoords.x = p_mesh->mTextureCoords[ 0 ][ v ].x;
				vertex._texCoords.y = p_mesh->mTextureCoords[ 0 ][ v ].y;
				// Tangent.
				vertex._tangent.x = p_mesh->mTangents[ v ].x;
				vertex._tangent.y = p_mesh->mTangents[ v ].y;
				vertex._tangent.z = p_mesh->mTangents[ v ].z;
				// Bitangent.
				vertex._bitangent.x = p_mesh->mBitangents[ v ].x;
				vertex._bitangent.y = p_mesh->mBitangents[ v ].y;
				vertex._bitangent.z = p_mesh->mBitangents[ v ].z;
			}
			else
			{
				vertex._texCoords.x = 0.f;
				vertex._texCoords.y = 0.f;
			}
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
		const aiMaterial * const mtl = p_scene->mMaterials[ p_mesh->mMaterialIndex ];
		Material				 material;
		if ( mtl == nullptr )
		{
			std::cerr << "[WARNING] - Loading mesh: " << meshName << std::endl;
			std::cerr << "=> Material undefined," << meshName << " assigned to default material" << std::endl;
		}
		else
		{
			material = _loadMaterial( mtl );
		}

		_nbTriangles += p_mesh->mNumFaces;
		_nbVertices += p_mesh->mNumVertices;

		_meshes.push_back( TriangleMesh( meshName, vertices, indices, material ) );

		if ( VERBOSE )
		{
			std::cout << "-- Done! "						  //
					  << indices.size() / 3 << " triangles, " //
					  << vertices.size() << " vertices." << std::endl;
		}
	}

	Material TriangleMeshModel::_loadMaterial( const aiMaterial * const p_mtl )
	{
		Material material;

		aiColor3D color;
		aiString  texturePath;
		Texture	  texture; // We suppose to have at most one texture per type.

		// ===================================================== AMBIENT
		if ( p_mtl->GetTextureCount( aiTextureType_AMBIENT ) > 0 ) // Texture ?
		{
			p_mtl->GetTexture( aiTextureType_AMBIENT, 0, &texturePath );
			texture = _loadTexture( texturePath, "ambient" );
			if ( texture._id != GL_INVALID_INDEX )
			{
				material._ambientMap	= texture;
				material._hasAmbientMap = true;
			}
		}
		else if ( p_mtl->Get( AI_MATKEY_COLOR_AMBIENT, color ) == AI_SUCCESS ) // else Material ?
		{
			material._ambient = Vec3f( color.r, color.g, color.b );
		}
		// =====================================================

		// ===================================================== DIFFUSE
		if ( p_mtl->GetTextureCount( aiTextureType_DIFFUSE ) > 0 ) // Texture ?
		{
			p_mtl->GetTexture( aiTextureType_DIFFUSE, 0, &texturePath );
			texture = _loadTexture( texturePath, "diffuse" );
			if ( texture._id != GL_INVALID_INDEX )
			{
				material._diffuseMap	= texture;
				material._hasDiffuseMap = true;
			}
		}
		else if ( p_mtl->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS ) // else Material ?
		{
			material._diffuse = Vec3f( color.r, color.g, color.b );
		}
		// =====================================================

		// ===================================================== SPECULAR
		if ( p_mtl->GetTextureCount( aiTextureType_SPECULAR ) > 0 ) // Texture ?
		{
			p_mtl->GetTexture( aiTextureType_SPECULAR, 0, &texturePath );
			texture = _loadTexture( texturePath, "specular" );
			if ( texture._id != GL_INVALID_INDEX )
			{
				material._specularMap	 = texture;
				material._hasSpecularMap = true;
			}
		}
		else if ( p_mtl->Get( AI_MATKEY_COLOR_SPECULAR, color ) == AI_SUCCESS ) // else Material ?
		{
			material._specular = Vec3f( color.r, color.g, color.b );
		}
		// =====================================================

		// ===================================================== SHININESS
		float shininess;
		if ( p_mtl->GetTextureCount( aiTextureType_SHININESS ) > 0 ) // Texture ?
		{
			p_mtl->GetTexture( aiTextureType_SHININESS, 0, &texturePath );
			texture = _loadTexture( texturePath, "shininess" );
			if ( texture._id != GL_INVALID_INDEX )
			{
				material._shininessMap	  = texture;
				material._hasShininessMap = true;
			}
		}
		else if ( p_mtl->Get( AI_MATKEY_SHININESS, shininess ) == AI_SUCCESS ) // else Material ?
		{
			material._shininess = shininess;
		}

		// =====================================================

		return material;
	}

	Texture TriangleMeshModel::_loadTexture( const aiString & p_path, const std::string & p_type )
	{
		const char * path = p_path.C_Str();

		if ( VERBOSE )
			std::cout << "Loading texture (" << p_type << "): " << path << std::endl;

		// Check if the texture has already been loaded.
		for ( size_t i = 0; i < _loadedTextures.size(); ++i )
		{
			if ( std::strcmp( _loadedTextures[ i ]._path.data(), path ) == 0 )
			{
				if ( VERBOSE )
				{
					std::cout << "-> Already loaded !" << std::endl;
				}
				if ( _loadedTextures[ i ]._type == p_type )
				{
					return _loadedTextures[ i ];
				}
				else // One texture can be used for more than one type.
				{
					Texture texture;
					texture._id	  = _loadedTextures[ i ]._id;
					texture._path = path;
					texture._type = p_type;
					return texture;
				}
			}
		}

		Texture texture;

		// Load the image and send it to the GPU.
		Image			  image;
		const std::string fullPath = _dirPath + path;

		if ( image.load( fullPath ) )
		{
			// Create a texture on the GPU.
			glCreateTextures( GL_TEXTURE_2D, 1, &texture._id );
			texture._path = path;
			texture._type = p_type;

			// Define formats.
			GLenum format		  = GL_INVALID_ENUM;
			GLenum internalFormat = GL_INVALID_ENUM;
			if ( image._nbChannels == 1 )
			{
				format		   = GL_RED;
				internalFormat = GL_R32F;
			}
			else if ( image._nbChannels == 2 )
			{
				format		   = GL_RG;
				internalFormat = GL_RG32F;
			}
			else if ( image._nbChannels == 3 )
			{
				format		   = GL_RGB;
				internalFormat = GL_RGB32F;
			}
			else
			{
				format		   = GL_RGBA;
				internalFormat = GL_RGBA32F;
			}

			// Setup the texture format.
			int nbNiveauMipmap = log2( glm::max( image._width, image._height ) );
			glTextureStorage2D( texture._id, nbNiveauMipmap, internalFormat, image._width, image._height );
			glTextureParameteri( texture._id, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTextureParameteri( texture._id, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTextureParameteri( texture._id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTextureParameteri( texture._id, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

			/*glTextureStorage2D( texture._id, 1, internalFormat, image._width, image._height );
			glTextureParameteri( texture._id, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTextureParameteri( texture._id, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTextureParameteri( texture._id, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTextureParameteri( texture._id, GL_TEXTURE_MAG_FILTER, GL_NEAREST );*/

			// Fill the texture.
			glTextureSubImage2D(
				texture._id, 0, 0, 0, image._width, image._height, format, GL_UNSIGNED_BYTE, image._pixels );

			glGenerateTextureMipmap( texture._id );
		}

		// Save loaded texture.
		_loadedTextures.emplace_back( texture );

		if ( VERBOSE )
			std::cout << "-> Done !" << std::endl;

		return texture;
	}

} // namespace M3D_ISICG
