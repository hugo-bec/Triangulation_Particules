#version 450

layout( location = 0 ) out vec4 fragColor;

in vec3 color;

uniform vec3 difuseColor;

void main() { 

	fragColor = vec4( color, 1.f ); 
}
