#include "../precompiled.h"
#include "Snail.hpp"
#include "../core/GLUtils.hpp"

Snail::~Snail()
{
	GLUtils::DeleteBuffer(ebo_);
	GLUtils::DeleteBuffer(vbo_);
	GLUtils::DeleteVAO(vao_);
}

void Snail::Init()
{
	position_ = glm::vec3(0.0f, 0.0f, 0.0f);
	yaw_ = 0.0f;

	forward_ = glm::vec3(0.0f, 0.0f, -1.0f);
	right_ = glm::vec3(1.0f, 0.0f, 0.0f);
	up_ = glm::vec3(0.0f, 1.0f, 0.0f);
	orientation_ = glm::mat4(1.0f);

	animation_time_ = 0.0f;
	creep_amount_ = 0.0f;
	is_moving_ = false;

	mode_ = Mode::Normal;
	retract_progress_ = 0.0f;
	r_was_pressed_ = false;

	speed_boost_timer_ = 0.0f;
	speed_boost_multiplier_ = 1.0f;

	ShellPhysics::Reset(shell_physics_);

	CreateBoxMesh();

	try
	{
		Logger::Log("Loading slug model: " + std::string(Config::slug_model_path));
		Logger::Log("Loading shell model: " + std::string(Config::shell_model_path));

		slug_object_ = std::make_unique<Object>(Config::slug_model_path);
		shell_object_ = std::make_unique<Object>(Config::shell_model_path);
	}
	catch (const std::exception& e)
	{
		Logger::Log(
			"Failed to load snail objects: " + std::string(e.what())
			+ " (falling back to placeholder geometry)",
			Logger::LogLevel::ERROR
		);

		slug_object_.reset();
		shell_object_.reset();
	}
}

void Snail::CreateBoxMesh()
{
	vertices_ = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f, 0.0f,

		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f
	};

	indices_ = {
		0, 1, 2, 2, 3, 0,
		4, 6, 5, 6, 4, 7,
		8, 9, 10, 10, 11, 8,
		12, 14, 13, 14, 12, 15,
		16, 17, 18, 18, 19, 16,
		20, 22, 21, 22, 20, 23
	};

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ebo_);

	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(vertices_.size() * sizeof(float)),
		vertices_.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(indices_.size() * sizeof(unsigned int)),
		indices_.data(),
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		reinterpret_cast<void*>(0)
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		reinterpret_cast<void*>(3 * sizeof(float))
	);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Snail::Update(float dt, GLFWwindow* window, const Terrain& terrain)
{
	if (!window)
	{
		return;
	}

	dt = glm::clamp(dt, 0.0f, 1.0f / 20.0f);

	HandleInput(dt, window);
	UpdateRetractAnimation(dt);

	if (speed_boost_timer_ > 0.0f)
	{
		speed_boost_timer_ -= dt;

		if (speed_boost_timer_ <= 0.0f)
		{
			speed_boost_timer_ = 0.0f;
			speed_boost_multiplier_ = 1.0f;
		}
	}

	if (mode_ == Mode::Shell)
	{
		UpdateShellPhysics(dt, window, terrain);
	}
	else
	{
		UpdateNormalMovement(dt, window);
	}

	if (mode_ != Mode::Shell)
	{
		ClampToTerrainBounds();

		const float terrainHeight =
			terrain.GetHeightAt(position_.x, position_.z);

		const float shellHeightOffset = shell_radius_;
		const float bodyHeightOffset = Config::snail_body_height_offset;

		const float targetOffset =
			IsShellOnly() ? shellHeightOffset : bodyHeightOffset;

		position_.y = terrainHeight + targetOffset;
	}
	else
	{
		ClampToTerrainBounds();
	}

	UpdateOrientationFromTerrain(terrain);
}

void Snail::HandleInput(float dt, GLFWwindow* window)
{
	(void)dt;

	const bool rIsPressed =
		glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

	if (rIsPressed && !r_was_pressed_)
	{
		if (mode_ == Mode::Normal)
		{
			mode_ = Mode::Retracting;
			ShellPhysics::Reset(shell_physics_);
		}
		else if (mode_ == Mode::Shell)
		{
			mode_ = Mode::Unretracting;
			ShellPhysics::Reset(shell_physics_);
		}
	}

	r_was_pressed_ = rIsPressed;
}

void Snail::UpdateNormalMovement(float dt, GLFWwindow* window)
{
	if (mode_ != Mode::Normal &&
		mode_ != Mode::Retracting &&
		mode_ != Mode::Unretracting)
	{
		return;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		yaw_ += turn_speed_ * dt;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		yaw_ -= turn_speed_ * dt;
	}

	forward_ =
		glm::normalize(glm::vec3(std::sin(yaw_), 0.0f, -std::cos(yaw_)));

	glm::vec3 moveDirection{ 0.0f };

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		moveDirection += forward_;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		moveDirection -= forward_;
	}

	is_moving_ = glm::length(moveDirection) > 0.001f;

	if (is_moving_)
	{
		moveDirection = glm::normalize(moveDirection);

		const float currentSpeed =
			move_speed_ * speed_boost_multiplier_;

		position_ += moveDirection * currentSpeed * dt;

		if (mode_ == Mode::Normal)
		{
			animation_time_ += dt * 8.0f;
		}
	}

	creep_amount_ =
		mode_ == Mode::Normal && is_moving_
		? std::sin(animation_time_)
		: 0.0f;
}

glm::vec3 Snail::ReadShellInput(GLFWwindow* window) const
{
	glm::vec3 input(0.0f);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		input.x -= 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		input.x += 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		input.z -= 1.0f;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		input.z += 1.0f;
	}

	if (glm::length2(input) > 0.0001f)
	{
		input = glm::normalize(input);
	}

	return input;
}

ShellPhysics::Settings Snail::BuildShellPhysicsSettings() const
{
	ShellPhysics::Settings settings;

	settings.radius = Config::snail_shell_radius;
	settings.ground_offset = Config::shell_ground_offset;

	settings.gravity = Config::shell_gravity;

	settings.acceleration =
		Config::shell_acceleration * speed_boost_multiplier_;

	settings.air_control = Config::shell_air_control;

	settings.downhill_force = Config::shell_downhill_force;
	settings.uphill_drag = Config::shell_uphill_drag;

	settings.ground_friction = Config::shell_flat_friction;
	settings.slope_friction = Config::shell_slope_friction;
	settings.air_drag = Config::shell_air_drag;

	settings.max_speed =
		Config::shell_max_speed * speed_boost_multiplier_;

	settings.max_vertical_speed = Config::shell_max_vertical_speed;

	settings.bounce_restitution = Config::shell_bounce_restitution;
	settings.bounce_tangent_keep = Config::shell_bounce_tangent_keep;
	settings.min_bounce_speed = Config::shell_min_bounce_speed;

	settings.launch_speed = Config::shell_launch_speed;
	settings.launch_normal_up_dot = Config::shell_launch_normal_up_dot;
	settings.launch_boost = Config::shell_launch_boost;

	settings.snap_to_ground_distance = Config::shell_snap_to_ground_distance;
	settings.sleep_vertical_epsilon = Config::shell_sleep_vertical_epsilon;

	settings.substeps = Config::shell_physics_substeps;
	settings.world_bound = Config::snail_world_bound;

	return settings;
}

void Snail::UpdateShellPhysics(float dt, GLFWwindow* window, const Terrain& terrain)
{
	const glm::vec3 input = ReadShellInput(window);
	const ShellPhysics::Settings settings = BuildShellPhysicsSettings();

	ShellPhysics::Step(
		shell_physics_,
		position_,
		yaw_,
		input,
		dt,
		terrain,
		settings
	);

	forward_ =
		glm::normalize(glm::vec3(std::sin(yaw_), 0.0f, -std::cos(yaw_)));

	is_moving_ = ShellPhysics::GetSpeed(shell_physics_) > 0.05f;
	creep_amount_ = 0.0f;
}

void Snail::ClampToTerrainBounds()
{
	ShellPhysics::ClampToBounds(position_, shell_physics_.velocity, Config::snail_world_bound);
}

float Snail::GetTerrainSlopeAngle(const Terrain& terrain) const
{
	const glm::vec3 normal =
		terrain.GetNormalAt(position_.x, position_.z);

	const float d = glm::clamp(
		glm::dot(normal, glm::vec3(0.0f, 1.0f, 0.0f)),
		-1.0f,
		1.0f
	);

	return std::acos(d);
}

void Snail::UpdateRetractAnimation(float dt)
{
	if (mode_ == Mode::Retracting)
	{
		retract_progress_ += retract_speed_ * dt;

		if (retract_progress_ >= 1.0f)
		{
			retract_progress_ = 1.0f;
			mode_ = Mode::Shell;
			ShellPhysics::Reset(shell_physics_);
		}
	}
	else if (mode_ == Mode::Unretracting)
	{
		retract_progress_ -= retract_speed_ * dt;

		if (retract_progress_ <= 0.0f)
		{
			retract_progress_ = 0.0f;
			mode_ = Mode::Normal;
			ShellPhysics::Reset(shell_physics_);
		}
	}
}

void Snail::UpdateOrientationFromTerrain(const Terrain& terrain)
{
	if (mode_ == Mode::Shell && !shell_physics_.grounded)
	{
		up_ = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		up_ = terrain.GetNormalAt(position_.x, position_.z);
	}

	glm::vec3 projectedForward =
		forward_ - glm::dot(forward_, up_) * up_;

	if (glm::length(projectedForward) < 0.001f)
	{
		projectedForward =
			glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), up_);
	}

	projectedForward = glm::normalize(projectedForward);
	right_ = glm::normalize(glm::cross(projectedForward, up_));

	orientation_ = glm::mat4(1.0f);
	orientation_[0] = glm::vec4(right_, 0.0f);
	orientation_[1] = glm::vec4(up_, 0.0f);
	orientation_[2] = glm::vec4(-projectedForward, 0.0f);
	orientation_[3] = glm::vec4(position_, 1.0f);
}

glm::mat4 Snail::BuildModelMatrix(
	const glm::vec3& localOffset,
	const glm::vec3& localScale
) const
{
	glm::vec3 worldOffset =
		right_ * localOffset.x +
		up_ * localOffset.y -
		forward_ * localOffset.z;

	glm::mat4 model = orientation_;
	model[3] = glm::vec4(position_ + worldOffset, 1.0f);

	model = glm::scale(model, localScale);

	return model;
}

glm::mat4 Snail::ComputeBodyModelMatrix() const
{
	const float bodyVisibility = 1.0f - retract_progress_;

	const glm::vec3 bodyOffset = glm::mix(
		Config::slug_body_normal_offset,
		Config::slug_body_retracted_offset,
		retract_progress_
	);

	const float retractScale = glm::max(0.08f, bodyVisibility);

	const glm::vec3 bodyScale = glm::vec3(
		Config::slug_body_draw_scale,
		Config::slug_body_draw_scale * retractScale,
		Config::slug_body_draw_scale * retractScale
	);

	return BuildModelMatrix(bodyOffset, bodyScale);
}

glm::mat4 Snail::ComputeShellModelMatrix() const
{
	const float shellPulse =
		1.0f + Config::shell_retract_pulse * retract_progress_;

	glm::mat4 shellModel = BuildModelMatrix(
		Config::shell_draw_offset,
		glm::vec3(Config::shell_draw_scale) * shellPulse
	);

	if (mode_ == Mode::Shell)
	{
		shellModel *= glm::mat4_cast(shell_physics_.rotation);
	}

	return shellModel;
}

bool Snail::IsBodyVisible() const
{
	return retract_progress_ < 0.98f;
}

bool Snail::IsShellOnly() const
{
	return mode_ == Mode::Shell || mode_ == Mode::Retracting;
}

void Snail::Draw(const std::shared_ptr<Shader>& shader) const
{
	if (vao_ == 0)
	{
		return;
	}

	if (slug_object_ && shell_object_)
	{
		shader->Bind();

		shader->ResetRenderFlags();
		shader->SetBool(true, "uUseMaterial");

		shader->Unbind();

		if (IsBodyVisible())
		{
			slug_object_->DrawWithModelMatrix(shader, ComputeBodyModelMatrix());
		}

		shell_object_->DrawWithModelMatrix(shader, ComputeShellModelMatrix());

		shader->Bind();

		shader->ResetRenderFlags();

		shader->Unbind();

		return;
	}

	glBindVertexArray(vao_);

	const float bodyVisibility = 1.0f - retract_progress_;

	const float stretch = 1.0f + 0.08f * creep_amount_;
	const float squash = 1.0f - 0.05f * creep_amount_;

	if (IsBodyVisible())
	{
		const glm::vec3 bodyOffset = glm::mix(
			glm::vec3(0.0f, 0.05f, 0.0f),
			glm::vec3(0.0f, 0.22f, 0.25f),
			retract_progress_
		);

		const glm::vec3 bodyScale =
			glm::vec3(0.38f * squash, 0.18f, 0.80f * stretch) *
			glm::vec3(1.0f, bodyVisibility, bodyVisibility);

		const glm::mat4 bodyModel =
			BuildModelMatrix(bodyOffset, bodyScale);

		shader->Bind();
		shader->ResetRenderFlags();
		shader->SetBool(true, "uUseObjectColor");
		shader->SetVec3(glm::vec3(0.55f, 0.42f, 0.28f), "uObjectColor");
		shader->SetMat4(bodyModel, "uModel");

		glDrawElements(
			GL_TRIANGLES,
			static_cast<GLsizei>(indices_.size()),
			GL_UNSIGNED_INT,
			nullptr
		);

		shader->Unbind();
	}

	const float shellPulse =
		1.0f + 0.10f * retract_progress_;

	glm::mat4 shellModel = BuildModelMatrix(
		glm::vec3(0.0f, 0.34f, 0.25f),
		glm::vec3(0.42f, 0.42f, 0.42f) * shellPulse
	);

	if (mode_ == Mode::Shell)
	{
		shellModel *= glm::mat4_cast(shell_physics_.rotation);
	}

	shader->Bind();
	shader->ResetRenderFlags();
	shader->SetBool(true, "uUseObjectColor");
	shader->SetVec3(glm::vec3(0.28f, 0.16f, 0.08f), "uObjectColor");
	shader->SetMat4(shellModel, "uModel");

	glDrawElements(
		GL_TRIANGLES,
		static_cast<GLsizei>(indices_.size()),
		GL_UNSIGNED_INT,
		nullptr
	);

	shader->Unbind();

	shader->Bind();
	shader->ResetRenderFlags();
	shader->Unbind();

	glBindVertexArray(0);
}

void Snail::DrawDepth(const std::shared_ptr<Shader>& shader) const
{
	if (slug_object_ && shell_object_)
	{
		if (IsBodyVisible())
		{
			slug_object_->DrawDepthWithModelMatrix(shader, ComputeBodyModelMatrix());
		}

		shell_object_->DrawDepthWithModelMatrix(shader, ComputeShellModelMatrix());

		return;
	}

	if (vao_ == 0)
	{
		return;
	}

	glBindVertexArray(vao_);

	const float bodyVisibility = 1.0f - retract_progress_;

	if (IsBodyVisible())
	{
		const glm::vec3 bodyOffset = glm::mix(
			glm::vec3(0.0f, 0.05f, 0.0f),
			glm::vec3(0.0f, 0.22f, 0.25f),
			retract_progress_
		);

		const glm::vec3 bodyScale =
			glm::vec3(0.38f, 0.18f, 0.80f) *
			glm::vec3(1.0f, bodyVisibility, bodyVisibility);

		const glm::mat4 bodyModel =
			BuildModelMatrix(bodyOffset, bodyScale);

		shader->Bind();
		shader->ResetDepthFlags();
		shader->SetMat4(bodyModel, "modelMatrix");
		shader->SetMat4(bodyModel, "uModel");

		glDrawElements(
			GL_TRIANGLES,
			static_cast<GLsizei>(indices_.size()),
			GL_UNSIGNED_INT,
			nullptr
		);

		shader->Unbind();
	}

	const float shellPulse =
		1.0f + 0.10f * retract_progress_;

	glm::mat4 shellModel = BuildModelMatrix(
		glm::vec3(0.0f, 0.34f, 0.25f),
		glm::vec3(0.42f, 0.42f, 0.42f) * shellPulse
	);

	if (mode_ == Mode::Shell)
	{
		shellModel *= glm::mat4_cast(shell_physics_.rotation);
	}

	shader->Bind();
	shader->ResetDepthFlags();
	shader->SetMat4(shellModel, "modelMatrix");
	shader->SetMat4(shellModel, "uModel");

	glDrawElements(
		GL_TRIANGLES,
		static_cast<GLsizei>(indices_.size()),
		GL_UNSIGNED_INT,
		nullptr
	);

	shader->Unbind();

	glBindVertexArray(0);
}

void Snail::ApplySpeedBoost(float duration, float multiplier)
{
	speed_boost_timer_ =
		glm::max(speed_boost_timer_, duration);

	speed_boost_multiplier_ =
		glm::max(speed_boost_multiplier_, multiplier);

	if (mode_ == Mode::Shell)
	{
		glm::vec3 hv = ShellPhysics::HorizontalVelocity(shell_physics_);

		if (glm::length2(hv) > 0.01f)
		{
			hv = glm::normalize(hv);
			shell_physics_.velocity +=
				hv * (multiplier - 1.0f) * 2.0f;
		}
		else
		{
			shell_physics_.velocity +=
				forward_ * (multiplier - 1.0f) * 2.0f;
		}
	}
}

void Snail::SlowShell(float multiplier)
{
	const float m = glm::clamp(multiplier, 0.0f, 1.0f);

	shell_physics_.velocity *= m;
	shell_physics_.angular_velocity *= m;
}