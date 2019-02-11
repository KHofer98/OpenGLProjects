#version 330 core
 //normalized device space
const vec2 positions[4] = vec2[]
(
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2( 1.0, -1.0)
);

const vec2 uvs[4] = vec2[] 
(
    vec2(0.0,  1.0),
    vec2(0.0, 0.0),
    vec2( 1.0,  1.0),
    vec2( 1.0, 0.0)
);

out vec2 uv;

void main()
{
	uv = uvs[ gl_VertexID ];
    gl_Position = vec4(positions[gl_VertexID], -1.0, 1.0);
}