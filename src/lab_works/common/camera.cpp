#include "camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

namespace SIM_PART
{
	void Camera::moveFront( const float p_delta )
	{
		_position -= _invDirection * p_delta;
		_computeViewMatrix();
	}

	void Camera::moveRight( const float p_delta )
	{
		_position += _right * p_delta;
		_computeViewMatrix();
	}

	void Camera::moveUp( const float p_delta )
	{
		_position += _up * p_delta;
		_computeViewMatrix();
	}

	void Camera::_computeViewMatrix() { _viewMatrix = glm::lookAt( _position, _position - _invDirection, _up ); }

} // namespace M3D_ISICG
