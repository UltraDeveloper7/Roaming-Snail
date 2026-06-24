#include "../precompiled.h"
#include "Environment.hpp"
#include "../core/Loader.hpp"
#include "../core/GLUtils.hpp"

Environment::Environment() : fbo_{}, rbo_{},
cube_map_shader_(std::make_unique<Shader>(Config::cubemap_vertex_path, Config::cubemap_fragment_path))
{
    CreateCube();
    CreateBuffers();

    const glm::mat4 capture_projection = glm::perspective(glm::half_pi<float>(), 1.0f, Config::near_clip, Config::far_clip);
    const glm::mat4 capture_views[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    hdr_texture_ = Loader::LoadEnvironment(Config::hdr_path);
    cube_map_ = std::make_unique<Texture>(Config::cube_map_size, true);
    RenderCubeMap(capture_projection, capture_views);
}

Environment::~Environment()
{
    GLUtils::DeleteRenderbuffer(rbo_);
    GLUtils::DeleteFramebuffer(fbo_);
}

void Environment::Draw(const std::shared_ptr<Shader>& background_shader) const
{
    if (!cube_map_)
    {
        return;
    }

    GLint oldDepthFunc = GL_LESS;
    GLboolean oldDepthMask = GL_TRUE;
    GLint oldActiveTexture = GL_TEXTURE0;

    glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &oldActiveTexture);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    background_shader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_map_->GetId());
    background_shader->SetInt(0, "environmentMap");

    cube_->Bind();
    cube_->Draw();
    cube_->Unbind();

    background_shader->Unbind();

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glActiveTexture(oldActiveTexture);

    glDepthMask(oldDepthMask);
    glDepthFunc(oldDepthFunc);
}

void Environment::CreateBuffers()
{
    glGenFramebuffers(1, &fbo_);
    glGenRenderbuffers(1, &rbo_);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Config::cube_map_size, Config::cube_map_size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Environment capture framebuffer incomplete");
    }
}

void Environment::CreateCube()
{
    std::vector vertices
    {
        Vertex(glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{0.0f,  0.0f, -1.0f}, glm::vec2{0.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f, -1.0f}, glm::vec3{0.0f,  0.0f, -1.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f, -1.0f}, glm::vec3{0.0f,  0.0f, -1.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f, -1.0f}, glm::vec3{0.0f,  0.0f, -1.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{0.0f,  0.0f, -1.0f}, glm::vec2{0.0f, 0.0f}),
        Vertex(glm::vec3{-1.0f,  1.0f, -1.0f}, glm::vec3{0.0f,  0.0f, -1.0f}, glm::vec2{0.0f, 1.0f}),

        Vertex(glm::vec3{-1.0f, -1.0f,  1.0f}, glm::vec3{0.0f,  0.0f,  1.0f}, glm::vec2{0.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f,  1.0f}, glm::vec3{0.0f,  0.0f,  1.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f,  1.0f}, glm::vec3{0.0f,  0.0f,  1.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f,  1.0f}, glm::vec3{0.0f,  0.0f,  1.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f,  1.0f,  1.0f}, glm::vec3{0.0f,  0.0f,  1.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f,  1.0f}, glm::vec3{0.0f,  0.0f,  1.0f}, glm::vec2{0.0f, 0.0f}),

        Vertex(glm::vec3{-1.0f,  1.0f,  1.0f}, glm::vec3{-1.0f, 0.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{-1.0f,  1.0f, -1.0f}, glm::vec3{-1.0f, 0.0f,  0.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{-1.0f, 0.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{-1.0f, 0.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f,  1.0f}, glm::vec3{-1.0f, 0.0f,  0.0f}, glm::vec2{0.0f, 0.0f}),
        Vertex(glm::vec3{-1.0f,  1.0f,  1.0f}, glm::vec3{-1.0f, 0.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),

        Vertex(glm::vec3{ 1.0f,  1.0f,  1.0f}, glm::vec3{1.0f,  0.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f, -1.0f}, glm::vec3{1.0f,  0.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f, -1.0f}, glm::vec3{1.0f,  0.0f,  0.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f, -1.0f}, glm::vec3{1.0f,  0.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f,  1.0f}, glm::vec3{1.0f,  0.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f,  1.0f}, glm::vec3{1.0f,  0.0f,  0.0f}, glm::vec2{0.0f, 0.0f}),

        Vertex(glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{0.0f, -1.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f, -1.0f}, glm::vec3{0.0f, -1.0f,  0.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f,  1.0f}, glm::vec3{0.0f, -1.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f, -1.0f,  1.0f}, glm::vec3{0.0f, -1.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f,  1.0f}, glm::vec3{0.0f, -1.0f,  0.0f}, glm::vec2{0.0f, 0.0f}),
        Vertex(glm::vec3{-1.0f, -1.0f, -1.0f}, glm::vec3{0.0f, -1.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),

        Vertex(glm::vec3{-1.0f,  1.0f, -1.0f}, glm::vec3{0.0f,  1.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f,  1.0f}, glm::vec3{0.0f,  1.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f, -1.0f}, glm::vec3{0.0f,  1.0f,  0.0f}, glm::vec2{1.0f, 1.0f}),
        Vertex(glm::vec3{ 1.0f,  1.0f,  1.0f}, glm::vec3{0.0f,  1.0f,  0.0f}, glm::vec2{1.0f, 0.0f}),
        Vertex(glm::vec3{-1.0f,  1.0f, -1.0f}, glm::vec3{0.0f,  1.0f,  0.0f}, glm::vec2{0.0f, 1.0f}),
        Vertex(glm::vec3{-1.0f,  1.0f,  1.0f}, glm::vec3{ 0.0f,  1.0f,  0.0}, glm::vec2{0.0f, 0.0f})
    };

    cube_ = std::make_unique<Mesh>(std::move(vertices));
}

void Environment::RenderCubeMapFaces(
    const Shader& shader,
    const glm::mat4 capture_views[6],
    GLuint targetTexture,
    int mipLevel
) const
{
    for (unsigned int i = 0; i < 6; ++i)
    {
        shader.SetMat4(capture_views[i], "view");
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, targetTexture, mipLevel
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube_->Bind();
        cube_->Draw();
        cube_->Unbind();
    }
}

void Environment::RenderCubeMap(const glm::mat4& capture_projection, const glm::mat4 capture_views[6]) const
{
    cube_map_shader_->Bind();
    cube_map_shader_->SetInt(1, "equirectangularMap");
    cube_map_shader_->SetMat4(capture_projection, "projection");
    glActiveTexture(GL_TEXTURE1);
    hdr_texture_->Bind();

    glViewport(0, 0, Config::cube_map_size, Config::cube_map_size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

    RenderCubeMapFaces(*cube_map_shader_, capture_views, cube_map_->GetId());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cube_map_->Bind();
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    cube_map_shader_->Unbind();
}
