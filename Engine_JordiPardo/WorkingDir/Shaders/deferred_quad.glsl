#ifdef DEFERRED_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

struct Light
{
    unsigned int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;


void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light
{
    unsigned int type;
    vec3 color;
    vec3 direction;
    vec3 position;
};

in vec2 vTexCoord;

uniform sampler2D uColor;
uniform sampler2D uNormals;
uniform sampler2D uPosition;
uniform sampler2D uDepth;

uniform vec3 uCameraPosition;
uniform Light lights[5];
uniform unsigned int lightCount;


uniform unsigned int renderTargetMode;

layout(location = 0) out vec4 oColor;



vec3 CalculateDirectionalLight(Light light, vec3 position, vec3 normal, vec3 viewDir);
vec3 CalculatePointLight(Light light, vec3 position, vec3 normal, vec3 viewDir);

void main()
{
    vec3 vColor      = vec3(texture(uColor,vTexCoord));
    vec3 vPosition   = vec3(texture(uPosition, vTexCoord));
    vec3 viewDir     = uCameraPosition - vPosition;
    vec3 vNormal     = normalize(vec3(texture(uNormals, vTexCoord)));
    float alpha      = texture(uNormals,vTexCoord).a;
    oColor = vec4(vec3(0.0f), 1.0f);

    switch(renderTargetMode)
    {
        case 0: oColor = vec4(vColor,1.0f); break;
        case 1: oColor = vec4(vNormal, 1.0f); break;
        case 2: oColor = vec4(vPosition, 1.0f); break;
        case 3: oColor = vec4(vec3(texture(uDepth, vTexCoord).r),1.0); break;
        case 4: 
        if(alpha>=0.1)
        {
            for(uint i = 0; i < lightCount; ++i)
             {
                 vec3 lightDir = normalize(lights[i].direction);
                 vec3 lightAmount = vec3(0.0f);

                if(lights[i].type == 0)
                {
                    lightAmount = CalculateDirectionalLight(lights[i], vPosition, normalize(vNormal), viewDir);
                }
                else
                {
                    lightAmount = CalculatePointLight(lights[i], vPosition, normalize(vNormal), viewDir);
                }
                oColor.rgb += lightAmount * vColor.rgb;
            }
          }
          else
          {
            oColor = vec4(vColor, 1.0f);
          }

        break;
    }
}

vec3 CalculateDirectionalLight(Light light, vec3 position, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(light.direction);

    float ambientStrenght = 0.2;
    vec3  ambient = ambientStrenght * light.color;

    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * light.color;

    float specularStrength = 0.1f;
    vec3 reflectDir = reflect(-lightDir, normal);
    
    viewDir = normalize(viewDir);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * light.color;

    return ambient + diffuse + specular;
}

vec3 CalculatePointLight(Light light, vec3 position, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = position - light.position;
    lightDir = normalize(-lightDir);

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    float distance = length(light.position - position);
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

    float ambientStrenght = 0.2;
    vec3  ambient = ambientStrenght * light.color;

    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = diff * light.color;

    float specularStrength = 0.1f;
    vec3 reflectDir = reflect(-lightDir, normal);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 2);
    vec3 specular = specularStrength * spec * light.color;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
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
