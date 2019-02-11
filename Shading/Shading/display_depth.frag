#version 330 core

layout ( location = 0 ) out vec4 fragmentColor;

uniform sampler2D depthTexture;
uniform float zNear;
uniform float zFar;
in vec4 lightclipSpace;
in vec2 uv;


void main()
{	
	 float z = texture( depthTexture, uv).r;
	float linearZ = ( 2 * zNear ) / ( zFar + zNear - z * ( zFar - zNear ) ); //z linear zurückrechnen, da sonst z zu nah an weiß

	fragmentColor = vec4( vec3( linearZ ), 1.0 ); //linearer ZWert gerendert
}