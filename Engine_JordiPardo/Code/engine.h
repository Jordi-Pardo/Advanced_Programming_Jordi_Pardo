//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#ifdef _DEBUG
#include <glad/glad.h>
#endif // _DEBUG
#ifndef _DEBUG
#include "../ThirdParty/glad/include/glad/glad.h";
#endif // !DEBUG

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::mat4 mat4;

enum class RenderTargetsMode
{
    ALBEDO,
    NORMALS
};

//Camera Setting

// Camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


enum Camera_State {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT
};


class Camera
{
public:
    Camera();
    Camera(glm::vec3 position, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float movementSpeed = 0.5f;
    float mouseSensitivity = 0.5f;
    float zoom = 0.5f;

    mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_State direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    void UpdateCameraVectors();
};

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Material 
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;
};
//SHADER ATTRIBUTES

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};


struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout vertexInputLayout;
};
//RELATE VBO WITH SHADER

struct Vao {
    GLuint handle;
    GLuint programHandle;
};
// VEO ATTRIBUTES 
struct VertexBufferAttribute {
    u8 location;
    u8 componentCount;
    u8 offset;

};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8 stride;
};
struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;
    std::vector<Vao> vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;
};

struct Model {
    u32 meshIdx;
    std::vector<u32> materialIdx;
};

struct Buffer
{
    GLuint handle;
    GLenum type;
    u32 size;
    u32 head;
    void* data;
};

enum class LightType
{
    LightType_Directional,
    LightType_Point
};


struct Entity
{
    mat4 worldMatrix;
    mat4 worldViewProjection;
    vec3 position;
    float metallic = 0.5f;
    float roughness = 0.5f;
    u32 modelIndex;
    u32 localParamsOffset;
    u32 localParamsSize;
};

struct Light
{
    LightType  type;
    vec3	   color;
    vec3       direction;
    vec3	   position;
    Entity	   entity;
};


enum Mode
{
    Mode_TexturedQuad,
    Mode_Count,
    Mode_Mesh
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    Camera camera;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    RenderTargetsMode currentRenderTargetMode;

    ivec2 displaySize;

    std::vector<Texture>    textures;
    std::vector<Program>    programs;
    std::vector<Material>   materials;
    std::vector<Mesh>       meshes;
    std::vector<Model>      models;
    std::vector<Entity> entities;
    std::vector<Light>    lights;

    Entity mainEntity;

    // program indices
    u32 texturedGeometryProgramIdx;

    u32 texturedMeshProgramIdx;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    GLint uniformBufferAlignment;
    u32 model;
    u32 sphereModel;
    u32 bufferHandle;
    Buffer cbuffer;


    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;
    GLuint texturedMeshProgram_uTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

void RenderModel(App* app,Entity model, Program texturedMeshProgram);

u32 LoadTexture2D(App* app, const char* filepath);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

u32 LoadModel(App* app, const char* filename);

u8 LoadProgramAttributes(Program& program);

Buffer CreateBuffer(u32 size, GLenum type, GLenum usage);

mat4 TransformScale(const vec3& scaleFactors);

mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors);
mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors);

//Assimp
//void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
//void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);
//void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
//u32 LoadModel(App* app, const char* filename);

