#include "TrackBallCamera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

namespace SIM_PART
{
	void TrackBallCamera::moveFront( const float p_delta )
	{
		latitude += p_delta;
		_updatePositionTrackball();
		_computeViewMatrix();
	}

	void TrackBallCamera::moveRight( const float p_delta )
	{
		longitude += p_delta;
		_updatePositionTrackball();
		_computeViewMatrix();
	}

	void TrackBallCamera::moveUp( const float p_delta )
	{
		rayon -= p_delta;
		_updatePositionTrackball();
		_computeViewMatrix();
	}

	void TrackBallCamera::_computeViewMatrix() { _viewMatrix = glm::lookAt( _position, Vec3f( 0.f ), _up ); }

	void TrackBallCamera::_updatePositionTrackball()
	{
		// std::cout << "latitude: " << latitude << ", longitude: " << longitude << std::endl;
		_position = Vec3f( rayon * sin( latitude / 2 ) * sin( longitude / 2 ),
						   rayon * cos( latitude / 2 ),
						   rayon * sin( latitude / 2 ) * cos( longitude / 2 ) );
	}

} // namespace M3D_ISICG