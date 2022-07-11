#version 450

// particle data
layout( location = 0 ) in vec3 aVertexPosition;
layout( location = 1 ) in vec3 aVertexColor;
layout( location = 2 ) in vec3 aVertexNormal;

out vec3 color;
out vec3 aVertexNormalOut;
out vec3 aVertexPositionOut;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
	color = aVertexColor;

	//compute position view space
	aVertexPositionOut = vec3( uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.f) );

	//compute normal view space
	vec3 vertexAddNormal = aVertexPosition + aVertexNormal;
	vec3 aVertexAddNormalOut = vec3( uViewMatrix * uModelMatrix * vec4(vertexAddNormal, 1.f) );
	aVertexNormalOut = aVertexAddNormalOut - aVertexPositionOut;


	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( aVertexPosition , 1.f );

}
