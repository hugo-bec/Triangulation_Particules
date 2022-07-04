#version 450

layout( location = 0 ) out vec4 fragColor;

in vec3 normal;

uniform vec3 difuseColor;

void main() { 

	fragColor = vec4( difuseColor, 1.f ); 
}
