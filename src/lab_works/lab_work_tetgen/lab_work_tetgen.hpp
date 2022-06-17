#pragma once
#include "GL/gl3w.h"
#include "common/TrackBallCamera.hpp"
#include "common/base_lab_work.hpp"
#include "common/camera.hpp"
#include "define.hpp"
#include <map>
#include <vector>
#include "imgui_impl_glut.h"
#include "tetgen.h"

#include "Point.hpp"
#include "CageMesh.hpp"
#include "DelaunayStructure.hpp"

namespace SIM_PART
{
	class LabWorkTetgen : public BaseLabWork
	{
	  public:
		LabWorkTetgen() : BaseLabWork() { _camera = new Camera(); }
		~LabWorkTetgen();

		bool init() override;
		void animate( const float p_deltaTime ) override;
		void render() override;

		void handleEvents( const SDL_Event & p_event ) override;
		void displayUI() override;

		void resize( const int p_width, const int p_height ) override;

	  private:
		bool _initProgram();
		void _initCamera();

		void _initBuffersParticules( DelaunayStructure * part_ptr );

		void tetrahedralize_particules( tetgenio * in, tetgenio * out );
		void _updateViewMatrix();
		void _updateProjectionMatrix();

		// Create a mesh representing a unit cage centerd in (0,0,0)
		void	   _createParticules();
		void	   _colorPoint();
		void	   init_particules( tetgenio * in );
		void	   update_particules( tetgenio * out );
		void	   update_points_tetras( tetgenio * out );
		void	   compute_neighbours();
		void	   compute_attract_points();

	  private:
		// ================ Scene data.
		CageMesh		_cage;
		DelaunayStructure		_particules;
		int				_nbparticules = 1000;
		std::vector<tetrasearch::Point*> list_points;
		std::vector<tetrasearch::Tetrahedron *> list_tetras;
		tetgenio								tetgenMesh;


		BaseCamera *	_camera;
		time_t			current_time;
		Vec3f			_dimCage = Vec3f(10);
		// ================

		// ================ Settings.
		Vec4f _bgColor = Vec4f( 0.8f, 0.8f, 0.8f, 1.f ); // Background color
		// Camera
		float _cameraSpeed		 = 0.1f;
		float _cameraSensitivity = 0.1f;
		float _fovy				 = 60.f;
		bool  _orthogonal_view	 = false;
		bool  _trackball_camera	 = false;
		// ================

		// ================ GL program data.
		GLuint _program				= GL_INVALID_INDEX;
		GLint _uModelMatrixLoc		= GL_INVALID_INDEX;
		GLint _uViewMatrixLoc		= GL_INVALID_INDEX;
		GLint _uProjectionMatrixLoc = GL_INVALID_INDEX;
		// ================

		static const std::string _shaderFolder;
	};
} // namespace SIM_PART
