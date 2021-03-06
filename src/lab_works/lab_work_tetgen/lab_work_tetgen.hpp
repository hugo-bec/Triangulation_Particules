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

#include "Particle.hpp"
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

		void create_particules( const unsigned int nb,
								Vec3f			   cage_dim,
								float			   size,
								float speed, std::vector<TriangleMeshModel *> & tmm_container );



		void handleEvents( const SDL_Event & p_event ) override;
		void displayUI() override;
		void resize( const int p_width, const int p_height ) override;

	  private:
		bool _initProgram();
		void _initCamera();

		void _updateViewMatrix();
		void _updateProjectionMatrix();

		// Create a mesh representing a unit cage centerd in (0,0,0)
		

	  private:
		// ================ Scene data.
		CageMesh				_cage;
		DelaunayStructure		_dstructure;
		std::vector<Particle *>	_particules;
		

		// ================ Settings Camera.
		Vec4f _bgColor = Vec4f( 0.8f, 0.8f, 0.8f, 1.f ); // Background color
		// Camera
		BaseCamera * _camera;
		float _cameraSpeed		 = 0.1f;
		float _cameraSensitivity = 0.1f;
		float _fovy				 = 60.f;
		bool  _orthogonal_view	 = false;
		bool  _trackball_camera	 = false;
		bool  _attract_mode		 = false;
		bool  _diffusion_mode	 = true;
		bool  _tetrahedra_mode	 = false;
		bool  _points_mode		 = false;
		bool  _mesh_mode		 = true;
		int	  _chosen_particule	 = 0;

		// ================ GL program data.
		GLuint _program				= GL_INVALID_INDEX;
		GLint _uViewMatrixLoc		= GL_INVALID_INDEX;
		GLint _uProjectionMatrixLoc = GL_INVALID_INDEX;

		time_t					 current_time;
		Chrono					 _chrono;
		static const std::string _shaderFolder;
	};
} // namespace SIM_PART
