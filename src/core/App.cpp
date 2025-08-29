#include "../precompiled.h"
#include "App.hpp"
#include "../objects/CueBallMap.hpp"

App::App() :
	window_(std::make_unique<Window>()),
	text_renderer_(std::make_unique<TextRenderer>()),
	menu_(std::make_unique<Menu>(window_->GetWidth(), window_->GetHeight())),
	main_shader_(std::make_shared<Shader>(Config::vertex_path, Config::fragment_path)),
	background_shader_(std::make_shared<Shader>(Config::background_vertex_path, Config::background_fragment_path)),
	gui_shader_(std::make_shared<Shader>(Config::CueMap_vertex_path, Config::CueMap_fragment_path)),
	depthShader(std::make_shared<Shader>(Config::depth_vertex_path, Config::depth_fragment_path)),
	camera_(std::make_unique<Camera>()),
	cue_ball_map_(std::make_shared<CueBallMap>(*camera_, window_->GetGLFWWindow())),
	lightSpaceMatrices_{}
{
	//Logger::Init("log.txt");
	text_renderer_->Init();
	camera_->Init();
}

App::~App() {
	Logger::Close();
}

void App::Run()
{
	while (!window_->ShouldClose())
	{
		glfwPollEvents();

		if (window_->Resized())
			OnResize();

		OnUpdate();

		glfwSwapBuffers(window_->GetGLFWWindow());
	}
}

void App::OnUpdate()
{
	const double current_frame = glfwGetTime();
	delta_time_ = current_frame - last_frame_;
	last_frame_ = current_frame;

	HandleState();

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// make sure depth testing is ON before drawing the 3D scene
	glEnable(GL_DEPTH_TEST);

	if (world_)
	{
		camera_->UpdateViewMatrix(static_cast<float>(delta_time_));
		camera_->UpdateMain(main_shader_, *world_);

		environment_->Prepare();

		//-----------------------------------------------//
		// 1) Render all the shadow maps
		RenderShadowMap();

		// (2) Bind each shadow map & pass lightSpaceMatrix (array upload)
		main_shader_->Bind();

		// how many lights we actually use this frame
		int total_lights = Config::light_count + (int)world_->GetLights().size();
		int finalLightCount = std::min(total_lights, Config::max_shader_lights);

		// tell the shader the count
		main_shader_->SetInt(finalLightCount, "lightCount");

		// bind depth textures to TU 9.. and gather the units we used
		int units[Config::max_shader_lights];
		for (int i = 0; i < finalLightCount; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + 9 + i);
			glBindTexture(GL_TEXTURE_2D, environment_->depthMap[i]);
			units[i] = 9 + i;

			// upload the matching light-space matrix
			std::string matName = "lightSpaceMatrix[" + std::to_string(i) + "]";
			main_shader_->SetMat4(lightSpaceMatrices_[i], matName.c_str());
		}

		// set the WHOLE sampler array in one call (critical for some drivers)
		main_shader_->SetIntArray("shadowMap[0]", units, finalLightCount);

		main_shader_->Unbind();
		//-----------------------------------------------//

		world_->Update(static_cast<float>(delta_time_), !in_menu_);
		world_->Draw(main_shader_);

		camera_->UpdateBackground(background_shader_);
		environment_->Draw(background_shader_);

		// Update CueBallMap visibility based on camera view and ball movement
		bool isTopDownView = camera_->IsTopDownView();
		bool ballsAreMoving = world_->AreBallsInMotion();
		bool shouldBeVisible = isTopDownView && !ballsAreMoving;
		cue_ball_map_->SetVisible(shouldBeVisible);
		if (cue_ball_map_->IsVisible()) {
			cue_ball_map_->Draw();
			cue_ball_map_->HandleMouseInput(window_->GetGLFWWindow());
		}

	}

	if (in_menu_)
		menu_->Draw(world_ == nullptr, has_started_);

	// ---- text UI pass ----
	// draw text without depth, then restore it immediately
	GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	// Show current player
	if (world_) {
		const auto& players = world_->GetPlayers();
		const auto& current_player = players[world_->GetCurrentPlayerIndex()];
		menu_->AddText(0.0f, 0.95f, "Current Player: " + current_player.GetName(), 0.6f);

		// Show the shot clock
		float clockSec = world_->GetShotClock();
		menu_->AddText(0.0f, 0.90f, "Shot Clock: " + std::to_string((int)clockSec), 0.6f);

		//short message in the center 
		const std::string msg = world_->GetMessage();
		if (!msg.empty())
		{
			menu_->AddText(0.35f, 0.95f, msg, 0.75f);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	text_renderer_->Render(menu_->GetTexts());

	// Handle menu clicks
	if (in_menu_) {
		if (menu_->ConsumePlayClicked()) {
			in_menu_ = false;
			glfwSetInputMode(window_->GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if (!world_) {
				Load();
				has_started_ = true;            // mark that a game has started at least once
				world_->ResetPlayerIndex();
				last_frame_ = glfwGetTime();
			}
		}
		if (menu_->ConsumeResetClicked() && world_) {
			world_->Reset();
			world_->ResetGame();
		}
		if (menu_->ConsumeExitClicked()) {
			window_->SetCloseFlag();
		}
	}

	glDisable(GL_BLEND);

	if (depthWasEnabled) glEnable(GL_DEPTH_TEST);   // restore for next frame

	static bool key_was_pressed[10] = { false };
	GLFWwindow* w = window_->GetGLFWWindow();
	for (int key = GLFW_KEY_0; key <= GLFW_KEY_9; ++key) {
		int lightIndex = key - GLFW_KEY_0;
		bool isPressed = (glfwGetKey(w, key) == GLFW_PRESS);

		if (isPressed && !key_was_pressed[lightIndex]) {
			if (world_) {
				world_->ToggleLight(lightIndex);
			}
			key_was_pressed[lightIndex] = true;
		}
		else if (!isPressed && key_was_pressed[lightIndex]) {
			key_was_pressed[lightIndex] = false;
		}
	}
}


void App::OnResize() const
{
	const int new_width = window_->GetWidth(), new_height = window_->GetHeight();

	glViewport(0, 0, new_width, new_height);

	if (camera_)
		camera_->UpdateProjectionMatrix(new_width, new_height);

	menu_->Update(new_width, new_height);

	text_renderer_->UpdateProjectionMatrix(new_width, new_height);
	text_renderer_->Update();

	// Update CueBallMap window size
	if (cue_ball_map_)
		cue_ball_map_->UpdateWindowSize();

	window_->ResetResizedFlag();
}

void App::Load()
{
	world_ = std::make_unique<World>(cue_ball_map_, *camera_);
	environment_ = std::make_unique<Environment>();

	camera_->Init();
	world_->Init();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_MULTISAMPLE);
	glViewport(0, 0, window_->GetWidth(), window_->GetHeight());

	main_shader_->Bind();
	main_shader_->SetInt(1, "irradianceMap");
	main_shader_->SetInt(2, "prefilterMap");
	main_shader_->SetInt(3, "brdfLUT");
	main_shader_->SetInt(4, "material.diffuseMap");
	main_shader_->SetInt(5, "material.roughnessMap");
	main_shader_->SetInt(6, "material.normalMap");
	main_shader_->SetInt(7, "material.aoMap");
	main_shader_->SetInt(8, "material.metallicMap");
	main_shader_->Unbind();
}


void App::RenderShadowMap()
{
	// We'll produce an orthographic projection for each shadow:
	const float S = Config::shadow_extent;
	glm::mat4 lightProjection = glm::ortho(-S, S, -S, S, Config::near_plane, Config::far_plane);


	// We have 3 “virtual” (non‐physical) lights + however many physical are in World
	// total_lights = 3 + e.g. 10 => up to 13
	const auto& physicalLights = world_->GetLights(); // The 10 real bulbs
	const int total_lights = Config::light_count + static_cast<int>(physicalLights.size());

	// For safety, clamp to max_shader_lights so we don't exceed our array
	int finalLightCount = std::min(total_lights, Config::max_shader_lights);

	for (int i = 0; i < finalLightCount; i++)
	{
		// 1) Determine the position of this i-th light.
		glm::vec3 lightPos;
		bool isOn = true;  // We'll override if it's a physical light that is off

		if (i < Config::light_count)
		{
			// This is one of the 3 virtual lights
			// Example logic from Camera::UpdateMain:
			// e.g. float light_position_x = i % 2 ? 2.0f*i : -2.0f*i;
			// place them at (light_position_x, 2.0, 0.0)
			const float light_position_x = (i % 2) ? 2.0f * i : -2.0f * i;
			lightPos = glm::vec3(light_position_x, 2.0f, 0.0f);

			// If you want to treat them as always ON for shadows, keep isOn=true.
			// Or you can add some condition to skip them if you want.
		}
		else
		{
			// This is one of the "physical" lights in world_->GetLights()
			int physicalIndex = i - Config::light_count;

			// Safety check: skip if out of range
			if (physicalIndex >= static_cast<int>(physicalLights.size()))
				break;

			auto& lightPtr = physicalLights[physicalIndex];
			lightPos = lightPtr->GetPosition();
			isOn = lightPtr->IsOn();  // skip rendering shadows if off
		}

		// 2) Skip shadow pass if not ON
		if (!isOn)
			continue;

		// 3) Build the view & final matrix
		glm::mat4 lightView = glm::lookAt(
			lightPos,
			glm::vec3(0.0f, 0.0f, 0.0f), // e.g. looking at origin or center
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		// 4) Store it in our array, so we can pass it to main shader
		lightSpaceMatrices_[i] = lightSpaceMatrix;

		// 5) Use the depthShader to render depth into environment_->depthMap[i]
		depthShader->Bind();
		depthShader->SetMat4(lightSpaceMatrix, "lightSpaceMatrix");

		glViewport(0, 0, Config::shadow_width, Config::shadow_height);
		glBindFramebuffer(GL_FRAMEBUFFER, environment_->depthMapFBO[i]);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Draw the scene from this light's POV
		world_->Draw(depthShader);

		// unbind
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Finally, restore your window viewport
	glViewport(0, 0, window_->GetWidth(), window_->GetHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::HandleState()
{
	GLFWwindow* window = window_->GetGLFWWindow();

	// --- Edge-trigger for ESC ---
	static int prevEsc = GLFW_RELEASE;
	const int escNow = glfwGetKey(window, GLFW_KEY_ESCAPE);
	const bool escPressed = (escNow == GLFW_PRESS && prevEsc == GLFW_RELEASE);

	// Ask the menu whether any modal is open (settings/help)
	bool modalOpen = false;
	if (menu_) {
		// requires small getters in Menu.hpp (see below)
		modalOpen = menu_->IsSettingsOpen() || menu_->IsHelpOpen();
	}

	// -------------------------------
	// 1) In-game -> ESC opens the menu
	// -------------------------------
	if (!in_menu_) {
		if (escPressed) {
			in_menu_ = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			prevEsc = escNow;
			return;
		}
	}
	// --------------------------------------------
	// 2) In menu and game NOT started and NO modal
	//    -> ESC quits the application
	// --------------------------------------------
	else { // in_menu_ == true
		if (!has_started_ && escPressed && !modalOpen) {
			window_->SetCloseFlag();
			prevEsc = escNow;
			return;
		}
		// Note: when a modal is open, ESC is handled inside Menu.cpp to close it.
	}

	// -------------------------------------------
	// 3) Only handle camera toggle when NOT in menu
	// -------------------------------------------
	if (!in_menu_) {
		static int last_mouse_button_state = GLFW_RELEASE;
		const int mouse_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

		if (mouse_button_state == GLFW_PRESS && last_mouse_button_state == GLFW_RELEASE) {
			const bool cueMapVisible = (cue_ball_map_ && cue_ball_map_->IsVisible());
			const bool cueMapHit = (cue_ball_map_ && cue_ball_map_->IsWithinBounds());

			if (!cueMapVisible || !cueMapHit) {
				const bool newTopDown = !camera_->IsTopDownView();
				camera_->SetTopDownView(newTopDown);

				glfwSetInputMode(
					window,
					GLFW_CURSOR,
					newTopDown ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
				);
			}
		}

		last_mouse_button_state = mouse_button_state;
	}

	// remember ESC for next frame
	prevEsc = escNow;
}


