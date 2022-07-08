#version 450

// particle data
layout( location = 0 ) in vec3 aParticlePosition;
layout( location = 1 ) in vec3 aParticleColor;
layout( location = 2 ) in vec3 aParticleNormal;

out vec3 color;
out vec3 normal;
out vec3 aVertexPositionOut;
out vec3 vs_source_light;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
	color = aParticleColor;
	normal = aParticleNormal;

	aVertexPositionOut = vec3( uViewMatrix * uModelMatrix * vec4(aParticlePosition, 1.f) );

	vs_source_light = vec3( uViewMatrix * vec4(vec3(0), 1.f) );

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( aParticlePosition , 1.f );

}
