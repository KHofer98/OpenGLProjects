#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 32;
uniform Light light;
uniform vec3 viewPos;
uniform int i;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = vec3(0.0); // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
  //  for(int i = 0; i < NR_LIGHTS; ++i)
    
        // calculate distance between light source and current fragment
        float distance1 = length(light.Position - FragPos);
		float attenuation;
        if(distance1 < light.Radius)
        {
            // diffuse
            vec3 lightDir = normalize(light.Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = light.Color * spec * Specular;
            // attenuation
            attenuation = 1.0 / ((1.0 + light.Linear * distance1 + light.Quadratic * distance1 * distance1)*0.5);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    
    FragColor = vec4(lighting, 1.0);
}