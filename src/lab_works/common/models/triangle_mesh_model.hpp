#ifndef __TRIANGLE_MESH_MODEL_HPP__
#define __TRIANGLE_MESH_MODEL_HPP__

#include "base_model.hpp"
#include "triangle_mesh.hpp"
#include "utils/file_path.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace SIM_PART
{
	class TriangleMeshModel : public BaseModel
	{
	  public:
		TriangleMeshModel()			 = default;
		virtual ~TriangleMeshModel() = default;

		// Load a 3D model with Assimp.
		void load( const std::string & p_name, const FilePath & p_filePath );

		void load_with_model( const std::string & p_name, const TriangleMeshModel & origin_model );

		void render( const GLuint p_glProgram ) const override;

		void cleanGL() override;

	  private:
		void	 _loadMesh( const aiMesh * const p_mesh, const aiScene * const p_scene );
		Material _loadMaterial( const aiMaterial * const p_mtl );
		Texture	 _loadTexture( const aiString & p_path, const std::string & p_type );

	  public:
		std::vector<TriangleMesh> _meshes;		   // A model can contain several meshes.
		std::vector<Texture>	  _loadedTextures; // Avoid to load a texture more than once.
		// Some stats.
		int _nbTriangles = 0;
		int _nbVertices	 = 0;

		FilePath _dirPath = "";
	};
} // namespace M3D_ISICG

#endif // __TRIANGLE_MESH_MODEL_HPP__
