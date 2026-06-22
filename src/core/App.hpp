#pragma once

#include "../precompiled.h"
#include "../interface/Window.hpp"
#include "../interface/Camera.hpp"
#include "../interface/Menu.hpp"
#include "../interface/TextRenderer.hpp"
#include "../core/Shader.hpp"
#include "../core/Texture.hpp"
#include "../core/Environment.hpp"
#include "../world/Terrain.hpp"
#include "../world/Vegetation.hpp"
#include "../player/Snail.hpp"
#include "../rendering/SceneLighting.hpp"



class App
{
public:
	App();
	~App();

	App(const App&) = delete;
	App(App&&) = delete;
	App& operator=(const App&) = delete;
	App& operator=(App&&) = delete;

	// Main application loop. Owns the frame rhythm: update, render, swap.
	void Run();

private:
	// Per-frame simulation update for UI, camera, snail, vegetation, and timers.
	void OnUpdate();
	// Responds to window size changes and resizes projection/framebuffer resources.
	void OnResize();
	// Handles global controls such as pause, escape, and menu actions.
	void ProcessInput();
	// Draws the complete frame, including shadow pass, scene pass, UI, and post.
	void Render();
	// Loads the terrain albedo texture through the shared texture pipeline.
	void LoadTerrainTexture();
	// Renders depth-only geometry from the sun direction for shadow mapping.
	void RenderShadowPass();

private:
	// Window and camera are the outer runtime context for all rendering.
	std::unique_ptr<Window> window_ = nullptr;
	std::unique_ptr<Camera> camera_ = nullptr;

	// HDR skybox and image-based lighting resources.
	std::unique_ptr<Environment> environment_ = nullptr;
	std::shared_ptr<Shader> background_shader_ = nullptr;

	// Main material shader used by terrain and most scene objects.
	std::shared_ptr<Shader> terrain_shader_ = nullptr;

	// Gameplay/world systems.
	std::unique_ptr<Vegetation> vegetation_ = nullptr;

	std::unique_ptr<Terrain> terrain_ = nullptr;
	std::unique_ptr<Snail> snail_ = nullptr;

	// Directional light and shadow resources.
	std::unique_ptr<SceneLighting> lighting_ = nullptr;
	std::shared_ptr<Shader> depth_shader_ = nullptr;

	// Menu state and text rendering are separate from gameplay rendering.
	std::unique_ptr<Menu> menu_ = nullptr;
	std::unique_ptr<TextRenderer> text_renderer_ = nullptr;

	// High-level UI/game flow flags.
	bool has_started_ = false;
	bool paused_ = false;

	// Previous key states are used for edge-triggered toggles.
	int previous_pause_state_ = GLFW_RELEASE;
	int previous_escape_state_ = GLFW_RELEASE;

	std::shared_ptr<Texture> terrain_texture_;

	// Frame timing. delta_time_ drives movement in seconds.
	double delta_time_ = 0.0;
	double last_frame_ = 0.0;

	// Fullscreen post-processing shaders.
	std::shared_ptr<Shader> screen_shader_ = nullptr;
	std::shared_ptr<Shader> blur_shader_ = nullptr;

	// Off-screen scene framebuffer.
	GLuint scene_fbo_ = 0;
	GLuint scene_color_texture_ = 0;
	GLuint scene_depth_rbo_ = 0;

	// Ping-pong buffers used by blur/post effects.
	GLuint pingpong_fbo_[2]{ 0, 0 };
	GLuint pingpong_color_[2]{ 0, 0 };

	// Fullscreen quad used to draw post-processed textures.
	GLuint screen_vao_ = 0;
	GLuint screen_vbo_ = 0;

	// Post-processing resource lifecycle.
	void InitPostProcessing();
	void ResizePostProcessing(int width, int height);
	void DestroyPostProcessing();

	// Scene framebuffer binding helpers.
	void BeginSceneFramebuffer();
	void EndSceneFramebuffer();

	void RenderFullscreenQuad() const;
	void RenderPostProcess(bool blurBackground);

};
