#version 450

layout( location = 0 ) out vec4 fragColor;

in vec3 color;
in vec3 aVertexNormalOut;
in vec3 aVertexPositionOut;
in vec3 vs_source_light;

uniform vec3 ambientColor;
uniform vec3 difuseColor;


void main() 
{ 
	vec3 Li = normalize( vec3(0,0,0) - aVertexPositionOut );	//vecteur de la lumire emise

	vec3 eclairage_ambient = color * 0.2f;
	vec3 eclairage_diffus = max ( dot( normalize(aVertexNormalOut) , Li ), 0.f ) * color;

	vec3 col = eclairage_ambient + eclairage_diffus;

	fragColor = vec4( col, 1.f ); 
	
}
