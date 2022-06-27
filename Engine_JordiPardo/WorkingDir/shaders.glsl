///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;



void main()
{

	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition,1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture,vTexCoord);
}


#endif
#endif

#ifdef SHOW_TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vPosition; //In worldspace
out vec3 vNormal;   //In worldspace

uniform mat4 uWorldMatrix;
uniform mat4 uWorldViewProjectionMatrix;

void main()
{
	vTexCoord = aTexCoord;
	vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
	mat4 model = mat4(1.0f);
    //vNormal   = vec3(uWorldMatrix * vec4(aNormal, 0.0));
    vNormal = mat3(transpose(inverse(model))) * aNormal;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0f);
	//gl_Position.z = -gl_Position.z;
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here

in vec2 vTexCoord;
in vec3 vNormal;

uniform sampler2D uTexture;
uniform float alpha;

layout(location = 0) out vec4 oColor;

void main()
{
	//oColor = vec4(1,0,0,alpha); //texture(uTexture,vTexCoord);
	oColor = texture(uTexture,vTexCoord);
	//oColor = vec4(vNormal,1.0);
	//oColor = vec4(vTexCoord.y);
}


#endif
#endif

#ifdef SPHERE

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location=0) in vec3 aPosition;
layout(location=2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;

	float clippingScale = 8.0;

	gl_Position = vec4(aPosition,clippingScale);

	gl_Position.z = -gl_Position.z;
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	//oColor = texture(uTexture,vTexCoord);
	oColor = vec4(vTexCoord.y);
}


#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
