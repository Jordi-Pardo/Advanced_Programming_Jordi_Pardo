#ifdef FORWARD_SHADER

#if defined(VERTEX) ///////////////////////////////////////////////////

struct Light
{
    unsigned int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;  
out vec3 vViewDir; 

uniform mat4 uWorldMatrix;
uniform mat4 uWorldViewProjectionMatrix;
uniform vec3 uCameraPosition;
uniform Light lights[5];
out vec3 lightPos;

void main()
{
	lightPos = lights[0].position;
	vTexCoord = aTexCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	mat4 model = mat4(1.0f);
    //vNormal   = vec3(uWorldMatrix * vec4(aNormal, 0.0));
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vViewDir  = uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0f);
    //gl_Position = vec4(aPosition,8.0f);

	//gl_Position.z = -gl_Position.z;

}

#elif defined(FRAGMENT) ///////////////////////////////////////////////


// TODO: Write your fragment shader here
struct Light
{
    unsigned int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 vPosition;

uniform sampler2D uTexture;
uniform float alpha;
uniform Light lights[5];
uniform unsigned int lightCount;

layout(location = 0) out vec4 rt0; //Albedo 
layout(location = 1) out vec4 rt1; //Normals 
layout(location = 2) out vec4 rt2; //Position 
layout(location = 3) out vec4 rt3; //Final Render 

vec3 CalculateDirectionalLight(Light light, vec3 normal, vec3 viewDir);
vec3 CalculatePointLight(Light light, vec3 normal, vec3 viewDir);

void main()
{
    rt0 = texture(uTexture,vTexCoord);
    rt1 = vec4(vNormal,1.0f);
    rt2 = vec4(vPosition, 1.0);
    rt3 = vec4(vec3(0.0), 1.0);

    for(uint i = 0; i < lightCount; ++i)
    {
        vec3 lightDir = normalize(lights[i].direction);
        vec3 lightAmount = vec3(0.0f);

        if(lights[i].type == 0)
        {
            lightAmount = CalculateDirectionalLight(lights[i], normalize(vNormal), vViewDir);
        }
        else
        {
            lightAmount = CalculatePointLight(lights[i], normalize(vNormal), vViewDir);
        }
        rt3.rgb += lightAmount * rt0.rgb;
    }
}


vec3 CalculatePointLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = vPosition - light.position;
    lightDir = normalize(-lightDir);

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    float distance = length(light.position - vPosition);
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

    float ambientStrenght = 0.2;
    vec3  ambient = ambientStrenght * light.color;

    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * light.color;

    float specularStrength = 0.1f;
    vec3 reflectDir = reflect(-lightDir, vNormal);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 2);
    vec3 specular = specularStrength * spec * light.color;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}
vec3 CalculateDirectionalLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.direction);

    float ambientStrenght = 0.2;
    vec3  ambient = ambientStrenght * light.color;

    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * light.color;

    float specularStrength = 0.1f;
    vec3 reflectDir = reflect(-lightDir, vNormal);
    
    viewDir = normalize(viewDir);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * light.color;

    return ambient + diffuse + specular;
}

#endif
#endif


#ifdef SHOW_DEPTH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = vec4(vec3(texture(uTexture, vTexCoord).r), 1.0);
}

#endif
#endif



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
