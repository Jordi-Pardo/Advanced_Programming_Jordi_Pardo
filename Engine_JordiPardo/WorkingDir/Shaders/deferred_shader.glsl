#ifdef DEFERRED_SHADER

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

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 rt1; //Normals 
layout(location = 2) out vec4 rt2; //Position 


void main()
{
    oColor = texture(uTexture,vTexCoord);
    rt1 = vec4(vNormal,1.0f);
    rt2 = vec4(vPosition,1.0f);
}

#endif
#endif




// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
