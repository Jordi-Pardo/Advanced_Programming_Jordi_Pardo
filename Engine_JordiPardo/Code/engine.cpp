//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GLFW/glfw3.h>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;
    
    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void Init(App* app)
{
    app->currentRenderTargetMode = RenderTargetsMode::ALBEDO;


    app->renderMode = RenderMode::DEFERRED;


    //Directional
    app->lights.push_back(CreateLight(app, LightType::LightType_Directional, vec3(2.5f, 3.0f, 0.0f), vec3(0.2f, 0.250f, 0.8f), vec3(0.9f, 0.9f, 0.9f)));
    //app->lights.push_back(CreateLight(app, LightType::LightType_Directional, vec3(-2.9f, 2.75f, -2.0f), vec3(1.0f, 0.0f, 1.0f), vec3(0.9f, 0.5f, 0.5f)));

    //Point
    app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(-1.0f, 2.75, 2.2f), vec3(1.0f), vec3(0.0f, 0.2f, 0.0f)));
    app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(1.0f, 2.75, 2.2f), vec3(1.0f), vec3(0.5f, 0.2f, 0.5f)));
    app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(0.1f, 1.55, -0.2f), vec3(1.0f), vec3(0.33f, 0.2f, 0.05f)));

    // TODO: Initialize your resources here!
    // - vertex buffers


    const VertexV3V2 vertices[] = {
        {glm::vec3(-1.,-1.,0.0), glm::vec2(0.0,0.0) },
        {glm::vec3(1.,-1.,0.0), glm::vec2(1.0,0.0) },
        {glm::vec3(1.,1.,0.0), glm::vec2(1.0,1.0) },
        {glm::vec3(-1.,1.,0.0), glm::vec2(0.0,1.0) },
    };

    const u16 indices[] = {
        0,1,2,
        0,2,3
    };

    app->camera = Camera(vec3(0.25f, 1.25f, 6.75f));
    app->camera.yaw = -90.0f;
    app->camera.pitch = -10.0f;
    app->camera.UpdateCameraVectors();


    // - element/index buffers

    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    

    // - vaos
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);
    

    // - programs (and retrieve uniform indices)
    app->deferredQuadProgramIdx = LoadProgram(app, "Shaders/deferred_quad.glsl", "DEFERRED_QUAD");
    Program& texturedGeometryProgram = app->programs[app->deferredQuadProgramIdx];

    app->forwardQuadProgramIdx = LoadProgram(app, "Shaders/forward_quad.glsl", "FORWARD_QUAD");
    Program& forwardGeometryProgram = app->programs[app->forwardQuadProgramIdx];
    
    Entity entity;
    entity.position = vec3(0.0f, 0.0f, 0.0f);
    app->model = LoadModel(app, "Patrick/Patrick.obj");
    entity.metallic = 2.0f;
    entity.roughness = 2.75f;
    entity.modelIndex = app->model;
    app->mainEntity = entity;
    app->entities.push_back(entity);

    for (int i = 1; i < app->lights.size(); i++)
    {
        Entity entity2;
        entity2.position = app->lights[i].position;
        app->sphereModel = LoadModel(app, "Primitives/Sphere/sphere.obj");
        entity2.metallic = 0.0f;
        entity2.roughness = 0.0f;
        entity2.modelIndex = app->sphereModel;
        app->entities.push_back(entity2);
    }

    

    //Render
    app->forwardMeshProgramIdx = LoadProgram(app, "Shaders/forward_shader.glsl", "FORWARD_SHADER");
    Program& forwardMeshProgram = app->programs[app->forwardMeshProgramIdx];
    LoadProgramAttributes(forwardMeshProgram);

    //Geomtry
    app->deferredProgramIdx = LoadProgram(app, "Shaders/deferred_shader.glsl", "DEFERRED_SHADER");
    Program& deferredMeshProgram = app->programs[app->deferredProgramIdx];
    LoadProgramAttributes(deferredMeshProgram);

    //DEPTH
    app->depthProgramIdx = LoadProgram(app, "Shaders/depth_shader.glsl", "DEPTH_SHADER");
    Program& depthProgram = app->programs[app->depthProgramIdx];
    LoadProgramAttributes(depthProgram);


    //app->deferredQuadProgramIdx = LoadProgram(app, "Shaders/deferred_shader.glsl", "DEFERRED_QUAD");
    //Program& deferredQuadProgram = app->programs[app->deferredQuadProgramIdx];
    //LoadProgramAttributes(deferredQuadProgram);
    //app->programUniformTexture = glGetUniformLocation(deferredQuadProgram.handle, "uTexture");

    
    //app->sphereMeshProgramIdx = LoadProgram(app, "Shaders/deferred_shader.glsl", "FORWARD_SHADER");
    //Program& sphereMeshProgram = app->programs[app->sphereMeshProgramIdx];
    //LoadProgramAttributes(sphereMeshProgram);
    //texturedMeshProgram.vertexInputLayout.attributes.push_back({ 0,3 });
    //texturedMeshProgram.vertexInputLayout.attributes.push_back({ 2,2 });

    //GenerateQuad(app);

    // - textures

    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");




    //Framebuffers
    //Frame Buffer Handle

    glGenFramebuffers(1, &app->framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    //Color Attachment
    glGenTextures(1, &app->colorAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->colorAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0); 
    
    //Normal Attachment
    glGenTextures(1, &app->normalAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->normalAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);  

    //Position Attachment
    glGenTextures(1, &app->positionAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->positionAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);  

    //Final Render Attachment
    glGenTextures(1, &app->finalRenderAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->finalRenderAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Depth Attchment
    glGenTextures(1, &app->depthAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->depthAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, app->colorAttachmentHandle, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, app->normalAttachmentHandle, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, app->positionAttachmentHandle, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, app->finalRenderAttachmentHandle, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, app->depthAttachmentHandle, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, DrawBuffers); // "1" is the size of DrawBuffers


    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (framebufferStatus)
        {
        case GL_FRAMEBUFFER_UNDEFINED:						ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
        case GL_FRAMEBUFFER_UNSUPPORTED:					ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:		ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
        default:											ELOG("Unknown framebuffer status error") break;
        }
        return;
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::Text("OpenGL Version %s", glGetString( GL_VERSION));



    float cameraPosition[3] = { app->camera.position.x, app->camera.position.y, app->camera.position.z };
    ImGui::DragFloat3("Camera position", cameraPosition, 0.1f, -20000000000000000.0f, 200000000000000000000.0f);
    app->camera.position = vec3(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

    const char* renderModeBuffers[] = { "FORWARD", "DEFERRED" };
    if (ImGui::BeginCombo("Render Mode", renderModeBuffers[(u32)app->renderMode]))
    {
        for (u64 i = 0; i < IM_ARRAYSIZE(renderModeBuffers); ++i)
        {
            bool isSelected = (i == (u32)app->renderMode);
            if (ImGui::Selectable(renderModeBuffers[i], isSelected))
            {
                app->renderMode = (RenderMode)i;
            }
        }

        ImGui::EndCombo();
    }

    const char* renderTargets[] = { "ALBEDO","NORMALS","POSITION","DEPTH","FINAL RENDER"};
    if (ImGui::BeginCombo("RenderTargets", renderTargets[(u32)app->currentRenderTargetMode])) {
        for (u64 i = 0; i < IM_ARRAYSIZE(renderTargets); ++i)
        {
            bool isSelected = (i == (u32)app->currentRenderTargetMode);
            if (ImGui::Selectable(renderTargets[i], isSelected)) {
                app->currentRenderTargetMode = (RenderTargetsMode)i;
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::TreeNode("Entites")) {

        for (int i = 0; i < app->entities.size(); i++)
        {
            ImGui::PushID(i);
            ImGui::Text("Entities %i", i);

            Entity& entity = app->entities[i];
            float position[3] = { entity.position.x, entity.position.y, entity.position.z };
            ImGui::DragFloat3("Position", position, 0.1f, -20000000000000000.0f, 200000000000000000000.0f);
            entity.position = vec3(position[0], position[1], position[2]);

            ImGui::PopID();
        }

        ImGui::TreePop();
    }




    ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    //app->mainEntity.position = vec3(1.0, 0.0, 0.0);

    HandleInput(app);

    for (int i = 1; i < app->lights.size(); i++)
    {
        app->lights[i].position = app->entities[i].position;
    }

}

void Render(App* app)
{
    //Render on a framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    //Render Targets Buffers
    GLuint drawBuffers[] = { app->colorAttachmentHandle, app->normalAttachmentHandle, app->positionAttachmentHandle,app->finalRenderAttachmentHandle };
    glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

    //Clear color and depth and enable depth test
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //Render models
    Program programModel = app->programs[app->deferredProgramIdx];

    if (app->renderMode == RenderMode::FORWARD) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Forward Model Shader");
        programModel = app->programs[app->forwardMeshProgramIdx];
    }
    else {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Deferred Model Shader");
    }

    glUseProgram(programModel.handle);

    //LIGHTS
    PassLightsToCurrentProgram(programModel, app);

    //Camera to Shder
    PassCameraPositionToCurrentProgram(programModel, app);

    //Matrix
    float aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;
    float znear = 0.1f;
    float zfar = 1000.0f;

    mat4 projection = glm::perspective(glm::radians(app->camera.zoom), aspectRatio, znear, zfar);
    mat4 view = app->camera.GetViewMatrix();

    for (int i = 0; i < app->entities.size(); i++)
    {

        mat4 world = app->entities[i].worldMatrix;

        world = TransformPositionScale(app->entities[i].position, vec3(0.45f));

        int worldMatrixLocation = glGetUniformLocation(programModel.handle, "uWorldMatrix");
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, glm::value_ptr(world));

        mat4 worldViewProjection = projection * view * world;

        int worldProjectionMatrixLocation = glGetUniformLocation(programModel.handle, "uWorldViewProjectionMatrix");
        glUniformMatrix4fv(worldProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldViewProjection));

        RenderModel(app, app->entities[i], programModel);
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // TODO: Draw your textured quad here!
    DrawDice(app);

}

void PassCameraPositionToCurrentProgram(Program& programModel, App* app)
{
    int cameraLocation = glGetUniformLocation(programModel.handle, "uCameraPosition");
    glUniform3fv(cameraLocation, 1, glm::value_ptr(app->camera.position));
}

void PassLightsToCurrentProgram(Program& programModel, App* app)
{
    int lightsCountLocation = glGetUniformLocation(programModel.handle, "lightCount");
    glUniform1ui(lightsCountLocation, app->lights.size());

    int lightNum = 0;

    for (int i = 0; i < app->lights.size(); i++)
    {
        int location = -1;
        std::string posStr = "lights[" + std::to_string(lightNum) + "]";
        location = glGetUniformLocation(programModel.handle, (posStr + ".type").c_str());
        glUniform1ui(location, (u32)app->lights[i].type);
        location = glGetUniformLocation(programModel.handle, (posStr + ".color").c_str());
        glUniform3fv(location, 1, glm::value_ptr(app->lights[i].color));
        location = glGetUniformLocation(programModel.handle, (posStr + ".position").c_str());
        glUniform3fv(location, 1, glm::value_ptr(app->lights[i].position));
        location = glGetUniformLocation(programModel.handle, (posStr + ".direction").c_str());
        glUniform3fv(location, 1, glm::value_ptr(app->lights[i].direction));
        lightNum++;
    }
}

void DrawDice(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Dice Texture");
    // - set the viewport
    //glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    // - bind the program 
    Program programTexturedGeometry = app->programs[app->forwardQuadProgramIdx];
    if (app->renderMode == RenderMode::DEFERRED)
    {
        programTexturedGeometry = app->programs[app->deferredQuadProgramIdx];
    }

    if (app->renderMode == RenderMode::FORWARD && app->currentRenderTargetMode == RenderTargetsMode::DEPTH)
    {
        programTexturedGeometry = app->programs[app->depthProgramIdx];
    }


    glUseProgram(programTexturedGeometry.handle);
    glBindVertexArray(app->vao);

    if (app->renderMode == RenderMode::DEFERRED) {
        int renderModeLocation = glGetUniformLocation(programTexturedGeometry.handle, "renderTargetMode");
        glUniform1ui(renderModeLocation, (u32)app->currentRenderTargetMode);

        PassLightsToCurrentProgram(programTexturedGeometry, app);
        PassCameraPositionToCurrentProgram(programTexturedGeometry, app);
    }


    // - set the blending state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(app->programUniformTexture, 0);

    if (app->renderMode == RenderMode::FORWARD) {
        glActiveTexture(GL_TEXTURE0);
        switch (app->currentRenderTargetMode)
        {
        case RenderTargetsMode::ALBEDO:
            glBindTexture(GL_TEXTURE_2D, app->colorAttachmentHandle);
            break;
        case RenderTargetsMode::NORMALS:
            glBindTexture(GL_TEXTURE_2D, app->normalAttachmentHandle);
            break;
        case RenderTargetsMode::POSITION:
            glBindTexture(GL_TEXTURE_2D, app->positionAttachmentHandle);
            break;
        case RenderTargetsMode::DEPTH: 
            glBindTexture(GL_TEXTURE_2D, app->depthAttachmentHandle); 
            break;
        case RenderTargetsMode::FINAL_RENDER:
            glBindTexture(GL_TEXTURE_2D, app->finalRenderAttachmentHandle);
            break;
        default:
            break;
        }
        GLuint colorTextureLocation = glGetUniformLocation(programTexturedGeometry.handle, "uColor");
        glUniform1i(colorTextureLocation, 0);

    }
    else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->colorAttachmentHandle);
        GLuint colorLocation = glGetUniformLocation(programTexturedGeometry.handle, "uColor");
        glUniform1i(colorLocation, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, app->normalAttachmentHandle);
        GLuint normalsTextureLocation = glGetUniformLocation(programTexturedGeometry.handle, "uNormals");
        glUniform1i(normalsTextureLocation, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, app->positionAttachmentHandle);
        GLuint positionTextureLocation = glGetUniformLocation(programTexturedGeometry.handle, "uPosition");
        glUniform1i(positionTextureLocation, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, app->depthAttachmentHandle);
        GLuint depthTextureLocation = glGetUniformLocation(programTexturedGeometry.handle, "uDepth");
        glUniform1i(depthTextureLocation, 3);
    }

    //GLuint colorTextureLocation = glGetUniformLocation(programTexturedGeometry.handle, "uColor");
    //glUniform1i(colorTextureLocation, 0);   
    

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glPopDebugGroup();
}



void RenderModel(App* app, Entity entity,Program texturedMeshProgram)
{
    Model& model = app->models[entity.modelIndex];
    Mesh& mesh = app->meshes[model.meshIdx];

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
        glBindVertexArray(vao);

        u32 submeshMaterialIdx = model.materialIdx[i];
        Material& submeshMaterial = app->materials[submeshMaterialIdx];

        if (submeshMaterial.albedoTextureIdx < app->textures.size()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
            int textureLocation = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");
            glUniform1i(textureLocation, 0);
        }
        Submesh& submesh = mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices)
{
    std::vector<float> vertices;
    std::vector<u32> indices;

    bool hasTexCoords = false;
    bool hasTangentSpace = false;

    // process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            hasTexCoords = true;
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if (mesh->mTangents != nullptr && mesh->mBitangents)
        {
            hasTangentSpace = true;
            vertices.push_back(mesh->mTangents[i].x);
            vertices.push_back(mesh->mTangents[i].y);
            vertices.push_back(mesh->mTangents[i].z);

            // For some reason ASSIMP gives me the bitangents flipped.
            // Maybe it's my fault, but when I generate my own geometry
            // in other files (see the generation of standard assets)
            // and all the bitangents have the orientation I expect,
            // everything works ok.
            // I think that (even if the documentation says the opposite)
            // it returns a left-handed tangent space matrix.
            // SOLUTION: I invert the components of the bitangent here.
            vertices.push_back(-mesh->mBitangents[i].x);
            vertices.push_back(-mesh->mBitangents[i].y);
            vertices.push_back(-mesh->mBitangents[i].z);
        }
    }

    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // store the proper (previously proceessed) material for this mesh
    submeshMaterialIndices.push_back(baseMeshMaterialIndex + mesh->mMaterialIndex);

    // create the vertex format
    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
    vertexBufferLayout.stride = 6 * sizeof(float);
    if (hasTexCoords)
    {
        vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, vertexBufferLayout.stride });
        vertexBufferLayout.stride += 2 * sizeof(float);
    }
    if (hasTangentSpace)
    {
        vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 3, 3, vertexBufferLayout.stride });
        vertexBufferLayout.stride += 3 * sizeof(float);

        vertexBufferLayout.attributes.push_back(VertexBufferAttribute{ 4, 3, vertexBufferLayout.stride });
        vertexBufferLayout.stride += 3 * sizeof(float);
    }

    // add the submesh into the mesh
    Submesh submesh = {};
    submesh.vertexBufferLayout = vertexBufferLayout;
    submesh.vertices.swap(vertices);
    submesh.indices.swap(indices);
    myMesh->submeshes.push_back(submesh);
}

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory)
{
    aiString name;
    aiColor3D diffuseColor;
    aiColor3D emissiveColor;
    aiColor3D specularColor;
    ai_real shininess;
    material->Get(AI_MATKEY_NAME, name);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);

    myMaterial.name = name.C_Str();
    myMaterial.albedo = vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    myMaterial.emissive = vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
    myMaterial.smoothness = shininess / 256.0f;

    aiString aiFilename;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.albedoTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        material->GetTexture(aiTextureType_EMISSIVE, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.emissiveTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.specularTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.normalsTextureIdx = LoadTexture2D(app, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        material->GetTexture(aiTextureType_HEIGHT, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.bumpTextureIdx = LoadTexture2D(app, filepath.str);
    }

    //myMaterial.createNormalFromBump();
}

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessAssimpMesh(scene, mesh, myMesh, baseMeshMaterialIndex, submeshMaterialIndices);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], myMesh, baseMeshMaterialIndex, submeshMaterialIndices);
    }
}

u32 LoadModel(App* app, const char* filename)
{
    const aiScene* scene = aiImportFile(filename,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes |
        aiProcess_SortByPType);

    if (!scene)
    {
        ELOG("Error loading mesh %s: %s", filename, aiGetErrorString());
        return UINT32_MAX;
    }

    app->meshes.push_back(Mesh{});
    Mesh& mesh = app->meshes.back();
    u32 meshIdx = (u32)app->meshes.size() - 1u;

    app->models.push_back(Model{});
    Model& model = app->models.back();
    model.meshIdx = meshIdx;
    u32 modelIdx = (u32)app->models.size() - 1u;

    String directory = GetDirectoryPart(MakeString(filename));

    // Create a list of materials
    u32 baseMeshMaterialIndex = (u32)app->materials.size();
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        app->materials.push_back(Material{});
        Material& material = app->materials.back();
        ProcessAssimpMaterial(app, scene->mMaterials[i], material, directory);
    }

    ProcessAssimpNode(scene, scene->mRootNode, &mesh, baseMeshMaterialIndex, model.materialIdx);

    aiReleaseImport(scene);

    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        vertexBufferSize += mesh.submeshes[i].vertices.size() * sizeof(float);
        indexBufferSize += mesh.submeshes[i].indices.size() * sizeof(u32);
    }

    glGenBuffers(1, &mesh.vertexBufferHandle);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    u32 indicesOffset = 0;
    u32 verticesOffset = 0;

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        const void* verticesData = mesh.submeshes[i].vertices.data();
        const u32   verticesSize = mesh.submeshes[i].vertices.size() * sizeof(float);
        glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
        mesh.submeshes[i].vertexOffset = verticesOffset;
        verticesOffset += verticesSize;

        const void* indicesData = mesh.submeshes[i].indices.data();
        const u32   indicesSize = mesh.submeshes[i].indices.size() * sizeof(u32);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
        mesh.submeshes[i].indexOffset = indicesOffset;
        indicesOffset += indicesSize;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return modelIdx;
}
GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    //Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
        {
            return submesh.vaos[i].handle;
        }
    }

    GLuint vaoHandle = 0;

    //Create a new vao for this submesh/program
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

    for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;

        for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
        {
            if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
            {
                const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;  //attribute offset + vertex offset
                const u32 stride = submesh.vertexBufferLayout.stride;
                glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }

        assert(attributeWasLinked); //The submesh should provide an attribute for each vertex input
    }

    glBindVertexArray(0);

    //Store it in the list of vas for this submesh
    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}
u8 LoadProgramAttributes(Program& program)
{
    GLsizei attributeCount;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    for (u32 i = 0; i < attributeCount; ++i)
    {
        GLchar attributeName[128];
        GLsizei attributeNameLenght;
        GLint attributeSize;
        GLenum attributeType;
        glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName), &attributeNameLenght, &attributeSize, &attributeType, attributeName);

        u8 attributeLoacation = glGetAttribLocation(program.handle, attributeName);
        program.vertexInputLayout.attributes.push_back({ attributeLoacation, (u8)attributeSize });
    }

    return attributeCount;
}
mat4 TransformScale(const vec3& scaleFactors)
{
    mat4 transform = glm::scale(scaleFactors);
    return transform;
}

mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors)
{
    mat4 transform = glm::translate(pos);
    transform = glm::scale(transform, scaleFactors);
    return transform;
}

mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors)
{
    mat4 transform = glm::translate(pos);
    transform = glm::rotate(transform, glm::radians(90.0f), rotation);
    transform = glm::scale(transform, scaleFactors);
    return transform;
}

Camera::Camera()
{
    position = vec3(0.0f, 0.0f, 0.0f);
    front = vec3(0.0f, -1.0f, 0.0f);
    right = vec3(1.0f, 0.0f, 0.0f);
    up = worldUp = vec3(0.0f, 1.0f, 0.0f);
    movementSpeed = SPEED;
    mouseSensitivity = SENSITIVITY;
    zoom = ZOOM;
}

Camera::Camera(glm::vec3 _position, glm::vec3 _up, float _yaw, float _pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
{
    position = _position;
    worldUp = _up;
    yaw = _yaw;
    pitch = _pitch;
    UpdateCameraVectors();
}

mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

void Camera::ProcessKeyboard(Camera_State direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    if (direction == CAMERA_FORWARD)
        position += front * velocity;
    if (direction == CAMERA_BACKWARD)
        position -= front * velocity;
    if (direction == CAMERA_LEFT)
        position -= right * velocity;
    if (direction == CAMERA_RIGHT)
        position += right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    zoom -= (float)yoffset;
    if (zoom < 1.0f) { zoom = 1.0f; }
    if (zoom > 45.0f) { zoom = 45.0f; }
}

void Camera::UpdateCameraVectors()
{
    // calculate the new Front vector
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = glm::normalize(glm::cross(right, front));
}

Light CreateLight(App* app, LightType lightType, vec3 position, vec3 direction, vec3 color)
{
    Light light;
    light.type = lightType;
    light.position = position;
    light.color = color;
    light.direction = direction;

    Entity entity;
    entity.position = position;

    //if (lightType == LightType::LightType_Directional) { entity.modelIndex = app->directionalLightModel; }
    //else if (lightType == LightType::LightType_Point) { entity.modelIndex = app->sphereModel; }

    light.entity = entity;

    return light;
}

void HandleInput(App* app)
{
    if (app->input.keys[K_W] == BUTTON_PRESSED) {
        app->camera.ProcessKeyboard(Camera_State::CAMERA_FORWARD, app->deltaTime);
    }
    if (app->input.keys[K_A] == BUTTON_PRESSED) {
        app->camera.ProcessKeyboard(Camera_State::CAMERA_LEFT, app->deltaTime);
    }
    if (app->input.keys[K_S] == BUTTON_PRESSED) {
        app->camera.ProcessKeyboard(Camera_State::CAMERA_BACKWARD, app->deltaTime);
    }
    if (app->input.keys[K_D] == BUTTON_PRESSED) {
        app->camera.ProcessKeyboard(Camera_State::CAMERA_RIGHT, app->deltaTime);
    }

    if (app->input.mouseButtons[LEFT] == BUTTON_PRESSED || app->input.mouseButtons[RIGHT] == BUTTON_PRESSED)
    {
        app->camera.ProcessMouseMovement(app->input.mouseDelta.x, -app->input.mouseDelta.y);
    }

    //app->camera.ProcessMouseScroll(app->input.mouseDelta);
}


