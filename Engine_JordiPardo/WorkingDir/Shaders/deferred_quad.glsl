#ifdef DEFERRED_QUAD

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

uniform sampler2D uColor;
uniform sampler2D uNormals;
uniform sampler2D uPosition;
uniform sampler2D uDepth;



uniform unsigned int renderTargetMode;

layout(location = 0) out vec4 oColor;

void main()
{
    vec3 vPosition   = vec3(texture(uPosition, vTexCoord));

    switch(renderTargetMode)
    {
        case 0: oColor = texture(uColor, vTexCoord); break;
        case 1: oColor = texture(uNormals, vTexCoord); break;
        case 2: oColor = vec4(vPosition, 1.0f); break;
        case 3: oColor = vec4(vec3(texture(uDepth, vTexCoord).r),1.0);
    }
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
