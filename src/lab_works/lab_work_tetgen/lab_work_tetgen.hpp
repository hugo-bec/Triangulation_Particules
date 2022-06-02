#pragma once
#include "GL/gl3w.h"
#include "common/TrackBallCamera.hpp"
#include "common/base_lab_work.hpp"
#include "common/camera.hpp"
#include "define.hpp"
#include <map>
#include <vector>

namespace SIM_PART
{
	class LabWorkTetgen : public BaseLabWork
	{
		struct WireMesh
		{
			~WireMesh()
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

		struct Particules
		{
			~Particules()
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
			std::vector<Vec3f>		  _vertices;
			std::vector<unsigned int> _indices;
			Mat4f					  _transformation = MAT4F_ID;
			// ================

			// ================ GL data.
			GLuint _vao = GL_INVALID_INDEX; // Vertex Array Object
			GLuint _ebo = GL_INVALID_INDEX; // Element Buffer Object

			// Vertex Buffer Objects.
			GLuint _vboPoints = GL_INVALID_INDEX;
			// ================
		};

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

		void _initBuffersCage( WireMesh * cage_ptr );
		void _initBuffersParticules( Particules * part_ptr );

		void _updateViewMatrix();
		void _updateProjectionMatrix();

		// Create a mesh representing a unit cage centerd in (0,0,0)
		WireMesh _createCage();
		Particules _createParticules();

	  private:
		// ================ Scene data.
		WireMesh		_cage;
		Particules		_particules;
		int				_nbparticules = 100;

		BaseCamera *	_camera;
		time_t			current_time;
		//Vec3f			_dimCage = Vec3f(5, 2, 3.021);
		Vec3f			_dimCage = Vec3f(5);
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
