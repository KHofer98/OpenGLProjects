#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
const vec3 positions[8] = vec3[]
(
    vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0),
    vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0,  1.0),
	vec3(-1.0,  1.0, -1.0),
    vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0,  1.0)
);
uniform mat4 lightND2worldTf = mat4(1.0);
uniform mat4 world2cameraClipTf = mat4(1.0);
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	TexCoords=aTexCoords;
	gl_Position = projection * view * vec4(FragPos, 1.0);

//	/vec3 p = positions[ gl_VertexID ];
//	vec4 worldPosition = lightND2worldTf * vec4( FragPos, 1.0 );
//	vec3 vertexPosition = worldPosition.xyz / worldPosition.w;
//
//	gl_Position = world2cameraClipTf * vec4(vertexPosition, 1.0);
}