#version 450

layout( location = 0 ) in vec3 aVertexPosition;
layout( location = 1 ) in vec3 aVertexColor;

out vec3 color;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
	color = aVertexColor;

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4( aVertexPosition , 1.f );
}
