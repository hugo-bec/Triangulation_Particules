#version 450

layout( location = 0 ) out vec4 fragColor;

in vec3 color;
in vec3 aVertexNormalOut;
in vec3 aVertexPositionOut;

uniform vec3 ambientColor;
uniform vec3 difuseColor;

vec3 light_pos = vec3(0);

void main() 
{ 
	vec3 Li = normalize( light_pos - aVertexPositionOut );	//vecteur de la lumire emise

	vec3 eclairage_ambient = color * 0.2f;
	vec3 eclairage_diffus = max ( dot( normalize(aVertexNormalOut) , Li ), 0.f ) * color;

	vec3 col = mix(eclairage_ambient + eclairage_diffus, vec3(0.2), min(( -aVertexPositionOut.z - 5 ) / 20, 1)  );

	fragColor = vec4( col, 1.f ); 
	
}
