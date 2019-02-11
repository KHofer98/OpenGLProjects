#version 330 core
struct Material{
	sampler2D specular;
    sampler2D diffuse;
    float shininess;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

vec2 poissonDisk[64] = vec2[] (
vec2(-0.613392, 0.617481),
vec2(0.170019, -0.040254),
vec2(-0.299417, 0.791925),
vec2(0.645680, 0.493210),
vec2(-0.651784, 0.717887),
vec2(0.421003, 0.027070),
vec2(-0.817194, -0.271096),
vec2(-0.705374, -0.668203),
vec2(0.977050, -0.108615),
vec2(0.063326, 0.142369),
vec2(0.203528, 0.214331),
vec2(-0.667531, 0.326090),
vec2(-0.098422, -0.295755),
vec2(-0.885922, 0.215369),
vec2(0.566637, 0.605213),
vec2(0.039766, -0.396100),
vec2(0.751946, 0.453352),
vec2(0.078707, -0.715323),
vec2(-0.075838, -0.529344),
vec2(0.724479, -0.580798),
vec2(0.222999, -0.215125),
vec2(-0.467574, -0.405438),
vec2(-0.248268, -0.814753),
vec2(0.354411, -0.887570),
vec2(0.175817, 0.382366),
vec2(0.487472, -0.063082),
vec2(-0.084078, 0.898312),
vec2(0.488876, -0.783441),
vec2(0.470016, 0.217933),
vec2(-0.696890, -0.549791),
vec2(-0.149693, 0.605762),
vec2(0.034211, 0.979980),
vec2(0.503098, -0.308878),
vec2(-0.016205, -0.872921),
vec2(0.385784, -0.393902),
vec2(-0.146886, -0.859249),
vec2(0.643361, 0.164098),
vec2(0.634388, -0.049471),
vec2(-0.688894, 0.007843),
vec2(0.464034, -0.188818),
vec2(-0.440840, 0.137486),
vec2(0.364483, 0.511704),
vec2(0.034028, 0.325968),
vec2(0.099094, -0.308023),
vec2(0.693960, -0.366253),
vec2(0.678884, -0.204688),
vec2(0.001801, 0.780328),
vec2(0.145177, -0.898984),
vec2(0.062655, -0.611866),
vec2(0.315226, -0.604297),
vec2(-0.780145, 0.486251),
vec2(-0.371868, 0.882138),
vec2(0.200476, 0.494430),
vec2(-0.494552, -0.711051),
vec2(0.612476, 0.705252),
vec2(-0.578845, -0.768792),
vec2(-0.772454, -0.090976),
vec2(0.504440, 0.372295),
vec2(0.155736, 0.065157),
vec2(0.391522, 0.849605),
vec2(-0.620106, -0.328104),
vec2(0.789239, -0.419965),
vec2(-0.545396, 0.538133),
vec2(-0.178564, -0.596057)
);

uniform SpotLight spotLight;
uniform vec3 viewPos;
uniform Material material;

out vec4 FragColor;

in vec3 Normal; //Normalvektoren per Kreuzprodukt ausrechnen (hier manuell hinzugefügt)
in vec3 FragPos;
in vec2 TexCoords;
in vec4 lightclipSpace;

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float rand (in vec2 co);

uniform sampler2D depth_text;
uniform sampler2D flashlight;
uniform float zNear;
uniform float zFar;
uniform float slopeScale;
uniform float constantbias;
uniform float rad;
uniform float samples;


float lightdir_spotlight;

void main()
{

    // properties
	float rotated = rand(vec2(FragPos.x, FragPos.y));
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
	float shadow = 0.2;
	
	vec3 result = CalcSpotLight(spotLight, norm, FragPos, viewDir);    

	 vec2 uvs = lightclipSpace.xy/lightclipSpace.w;
	 uvs = 0.5*uvs + vec2(0.5);

	 float depth = (lightclipSpace.z/lightclipSpace.w) * 0.5 + 0.5;
	 vec3 textureflash = vec3(1.0);

	 vec2 texlSize = 1.0/textureSize(depth_text, 0);

	 if (uvs.x>0.0 && uvs.x<1.0 && uvs.y>0.0 && uvs.y<1.0)
	 {
	for (int i = 0; i<max(samples,64); i++)
	{
		 vec2 rotatedPoisson = poissonDisk[i] * mat2x2(cos(rotated), -sin(rotated), sin(rotated), cos(rotated)) * rad;

		 vec2 lookup = uvs + rotatedPoisson * texlSize;
		 float slopeScalebias = slopeScale * max(tan(acos(dot(norm, normalize(spotLight.position-FragPos)))), 0.01); //slopeScaledepthbias
		 float z = texture(depth_text, lookup).r + constantbias + slopeScalebias; //constant bias (+slopeScale)

		if (depth<z) shadow += 1.0;
	}

	shadow/=samples;

	}
	shadow = max(shadow,0.1);
	result *= vec3(shadow) * 0.3;
	FragColor = vec4(result, 1.0);;

   }

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	 vec2 uvs = lightclipSpace.xy/lightclipSpace.w;
	 uvs = 0.5*uvs + vec2(0.5);

    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = diff */* texture(material.diffuse, TexCoords).xyz * */texture( flashlight, uvs ).rgb;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= intensity;// * intensity;
    diffuse *= intensity;//attenuation * intensity;
    specular *=intensity;// attenuation * intensity;
    return (ambient + diffuse + specular);
}

float rand(in vec2 co)
{
 return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}