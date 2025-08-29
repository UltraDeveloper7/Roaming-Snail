#include "../precompiled.h"
#include "World.hpp"
#include "../Config.hpp"
#include "../interface/Camera.hpp"
#include "../core/Loader.hpp"
#include "Logger.hpp"


/**
 * Intersect the ray with plane y=0, which is the top mesh of your table.
 * The ball’s center must ultimately be at y=Ball::radius_, so we'll do that after.
 */
static std::pair<bool, glm::vec3> RayIntersectTablePlane(const glm::vec3& rayOrigin,
	const glm::vec3& rayDir)
{
	const float planeY = 0.0f; // table top is at y=0

	if (fabs(rayDir.y) < 1e-7f)
		return { false, glm::vec3(0.f) };

	float t = (planeY - rayOrigin.y) / rayDir.y;
	if (t < 0.f)
		return { false, glm::vec3(0.f) };

	glm::vec3 point = rayOrigin + t * rayDir;
	return { true, point };
}

World::World(std::shared_ptr<CueBallMap> cue_ball_map, Camera& camera) :
	table_(std::make_shared<Table>()),
	cue_(std::make_shared<Cue>(cue_ball_map)),
	camera_(camera),
	ceiling_(std::make_shared<Ceiling>(Config::ceiling_path, glm::vec3(0.0f, 1.48f, 0.04f), glm::vec3(0.4f), glm::vec3(0.0f, 1.0f, 0.0f)))
{

	for (int n = 0; n < 16; n++)
	{
		balls_.push_back(std::make_shared<Ball>(n));
		balls_[n]->Translate(glm::vec3{ 0.0f, Ball::radius_, 0.0f });
	}

    // mixing balls
	balls_[5].swap(balls_[8]);
	balls_[5].swap(balls_[15]);

	auto rd = std::random_device{};
	auto rng = std::default_random_engine{ rd() };
	std::shuffle(balls_.begin() + 1, balls_.end() - 1, rng);

	balls_[5].swap(balls_[15]);

	// Initialize lights
	InitializeLights();
}

void World::InitializeLights() {
	// Table bounds
	const float bound_x = Table::bound_x_; // Half-length along x-axis
	const float bound_z = Table::bound_z_; // Half-length along z-axis
	const float lightHeight = 0.2f;        // Height of the lights above the table

	// Positions for the lights
	std::array<glm::vec3, 10> lightPositions = {

		glm::vec3(-bound_x * 0.66f, lightHeight,  bound_z + 0.1f), // Left front
		glm::vec3(0.0f, lightHeight,  bound_z + 0.1f),             // Center front
		glm::vec3(bound_x * 0.66f, lightHeight,  bound_z + 0.1f),  // Right front
		glm::vec3(bound_x + 0.1f, lightHeight,  bound_z * 0.5f),   // Right side front
		glm::vec3(bound_x + 0.1f, lightHeight, -bound_z * 0.5f),   // Right side back
		glm::vec3(bound_x * 0.66f, lightHeight, -bound_z - 0.1f),  // Right back
		glm::vec3(0.0f, lightHeight, -bound_z - 0.1f),             // Center back
		glm::vec3(-bound_x * 0.66f, lightHeight, -bound_z - 0.1f), // Left back
		glm::vec3(-bound_x - 0.1f, lightHeight, -bound_z * 0.5f),  // Left side back
		glm::vec3(-bound_x - 0.1f, lightHeight,  bound_z * 0.5f)  // Left side front


	};

	// Initialize the lights
	const float cameraHeightOffset = 0.9f;
	const glm::vec3 lightScale(2.5f, 2.5f, 2.5f); // Example scale for the lights
	for (auto pos : lightPositions) {
		pos.y += cameraHeightOffset;
		auto light = std::make_shared<Light>(
			Config::lightbulb_path,
			pos,
			glm::vec3(0.0f, 0.0f, 0.0f)  // Default black color for each light

		);
		light->SetPosition(pos); // Set the position of the light
		light->SetScale(lightScale); // Set the scale of the light


		lights_.push_back(light);
	}
}

void World::ToggleLight(int index) {
	if (index >= 0 && index < lights_.size()) {
		lights_[index]->Toggle();
	}
}

void World::Draw(const std::shared_ptr<Shader>& shader) const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	table_->Draw(shader);

	if (AreBallsInMotion())
		cue_->PlaceAtBall(balls_[0]);
	else
		cue_->Draw(shader);

	for (const auto& ball : balls_)
		if (ball->IsDrawn())
			ball->Draw(shader);

	// Draw Ceiling
	ceiling_->Draw(shader);

	// Draw lights
	for (const auto& light : lights_) {
		light->Draw(shader);
	}

	glDisable(GL_BLEND);
}


void World::Update(float dt, bool in_game)
{
	state_.TickMessage(dt);
	if (state_.IsGameOver()) return;


	if (in_game) {
		if (state_.TickShotClock(dt)) {
			state_.SetMessage("Foul! Shot clock expired.", 1.2f);
			state_.SetBallInHand(true);
			state_.SwitchTurn(balls_[0]->IsDrawn());
		}


		if (state_.BallInHand()) {
			PlaceCueBallWithMouse();
		}
		else {
			cue_->HandleShot(balls_[0], dt);
		}
	}


	static bool wasMoving = false;
	bool nowMoving = AreBallsInMotion();
	if (!wasMoving && nowMoving) {
		state_.StartNewTurn();
	}
	wasMoving = nowMoving;


	for (int i = 0; i < (int)balls_.size(); ++i) {
		balls_[i]->Roll(dt);


		if (balls_[i]->IsInHole(table_->GetHoles(), Table::hole_radius_))
			HandleHolesFall(i);
		else
			HandleBoundsCollision(i);


		if (!balls_[i]->IsInHole(table_->GetHoles(), Table::hole_radius_))
			balls_[i]->translation_.y = Ball::radius_;


		HandleBallsCollision(i);
	}


	if (!AreBallsInMotion()) {
		if (state_.CheckRulesPending()) {
			rules_.EvaluateEndOfShot(balls_, state_);
			state_.SetCheckRulesPending(false);
		}
	}
	else {
		state_.SetCheckRulesPending(true);
	}
}


bool World::AreBallsInMotion() const {
	for (const auto& b : balls_) if (b->IsInMotion()) return true; return false;
}


void World::Init() const {
	balls_[0]->Translate(glm::vec3(0.8f, 0.0f, 0.0f));
	cue_->translation_ = glm::vec3(0.0f);
	cue_->angle_ = 0.0f;
	cue_->Translate(glm::vec3(0.8f + Ball::radius_ + Config::min_change, Ball::radius_, 0.0f));
	cue_->Rotate(glm::vec3(-0.1f, 1.0f, 0.0f), glm::pi<float>());


	balls_[1]->Translate(glm::vec3(-0.8f + 2.0f * glm::root_three<float>() * Ball::radius_, 0.0f, 0.0f));


	glm::vec3 temp = balls_[1]->translation_;
	int index = 2;
	for (int i = 0; i < 4; ++i) {
		temp.x -= glm::root_three<float>() * Ball::radius_;
		temp.z -= Ball::radius_;
		for (int j = 0; j < i + 2; ++j) {
			balls_[index]->Translate(glm::vec3(temp.x, 0.0f, temp.z + j * (Ball::radius_ * 2.0f)));
			++index;
		}
	}
}


void World::Reset() const {
	for (const auto& ball : balls_) { ball->TakeFromHole(); ball->SetDrawn(true); }
	Init();
}


void World::ResetGame() {
	for (auto& p : state_.Players()) p.ResetScore();
	state_.SetShotClock(GameState::SHOT_CLOCK_MAX);
	state_.ResetPlayerIndex();
}


void World::HandleBallsCollision(const int number)
{
	for (int j = number + 1; j < (int)balls_.size(); ++j) {
		const auto& a = balls_[number];
		const auto& b = balls_[j];


		bool cueCollisionHappened = false;
		if (number == 0 || j == 0) {
			float distance = glm::length(a->translation_ - b->translation_);
			float collisionDistance = 2.0f * Ball::radius_;
			if (distance <= (collisionDistance + 1e-4f)) cueCollisionHappened = true;
		}


		a->CollideWith(b);


		if (cueCollisionHappened && (number == 0 || j == 0)) {
			int obj = (number == 0 ? j : number);
			if (obj != 0) state_.MarkCueContact(obj);
		}
	}
}


void World::HandleHolesFall(const int number) 
{
	if (!AreBallsInMotion()) {
		// pocket the ball
		balls_[number]->SetDrawn(false);

		// if it’s the cue ball (index 0) → foul + Ball in Hand
		if (number == 0) {
			state_.SetBallInHand(true);
			state_.SetMessage("Foul! Scratch — ball in hand.", 1.2f);
			// (we don’t switch turns here; end-of-shot logic will do it)
		}
		return;
	}

	// still moving → keep the existing sink animation/physics
	balls_[number]->HandleGravity(Table::hole_bottom_);

	const auto p2 = glm::vec2(balls_[number]->translation_.x, balls_[number]->translation_.z);
	const auto h2 = glm::vec2(balls_[number]->GetHole().x, balls_[number]->GetHole().z);

	const glm::vec2 dir = glm::normalize(p2 - h2);
	const float distance = glm::distance(p2, h2);

	if (distance > Table::hole_radius_ - Ball::radius_)
		balls_[number]->BounceOffHole(-dir, Table::hole_radius_);
}


void World::HandleBoundsCollision(const int number) const
{
	const auto ball_pos = balls_[number]->translation_;
	constexpr auto hole_edge_z = 0.7f - Table::hole_radius_ - Ball::radius_;
	constexpr auto hole_edge_x = 1.35f - Table::hole_radius_ - Ball::radius_;

	// X-bounds
	if (ball_pos.x >= Table::bound_x_ && ball_pos.z < hole_edge_z && ball_pos.z > -hole_edge_z)
	{
		balls_[number]->BounceOffBound(glm::vec3(-1.0f, 0.0f, 0.0f), Table::bound_x_, Table::bound_z_);
	}
	else if (ball_pos.x <= -Table::bound_x_ && ball_pos.z < hole_edge_z && ball_pos.z > -hole_edge_z)
	{
		balls_[number]->BounceOffBound(glm::vec3(1.0f, 0.0f, 0.0f), Table::bound_x_, Table::bound_z_);
	}
	// Z-bounds
	else if (ball_pos.z >= Table::bound_z_ &&
		((ball_pos.x < hole_edge_x && ball_pos.x > Table::hole_radius_) ||
			(ball_pos.x > -hole_edge_x && ball_pos.x < -Table::hole_radius_)))
	{
		balls_[number]->BounceOffBound(glm::vec3(0.0f, 0.0f, -1.0f), Table::bound_x_, Table::bound_z_);
	}
	else if (ball_pos.z <= -Table::bound_z_ &&
		((ball_pos.x < hole_edge_x && ball_pos.x > Table::hole_radius_) ||
			(ball_pos.x > -hole_edge_x && ball_pos.x < -Table::hole_radius_)))
	{
		balls_[number]->BounceOffBound(glm::vec3(0.0f, 0.0f, 1.0f), Table::bound_x_, Table::bound_z_);
	}
}

/**
 * If user tries to put ball into hole radius, push it out
 */
glm::vec3 World::ClampCueBallPosition(const glm::vec3& desiredPos) const
{
	glm::vec3 pos = desiredPos;

	// 1) Check corners/pockets with your existing logic
	float hole_edge_x = (Table::bound_x_ - Table::hole_radius_ - Ball::radius_);
	float hole_edge_z = (Table::bound_z_ - Table::hole_radius_ - Ball::radius_);

	// Right cushion region
	if (pos.x >= Table::bound_x_ &&
		(pos.z < hole_edge_z && pos.z > -hole_edge_z))
	{
		pos.x = Table::bound_x_;
	}
	else if (pos.x <= -Table::bound_x_ &&
		(pos.z < hole_edge_z && pos.z > -hole_edge_z))
	{
		pos.x = -Table::bound_x_;
	}
	else if (pos.z >= Table::bound_z_ &&
		((pos.x < hole_edge_x && pos.x > Table::hole_radius_) ||
			(pos.x > -hole_edge_x && pos.x < -Table::hole_radius_)))
	{
		pos.z = Table::bound_z_;
	}
	else if (pos.z <= -Table::bound_z_ &&
		((pos.x < hole_edge_x && pos.x > Table::hole_radius_) ||
			(pos.x > -hole_edge_x && pos.x < -Table::hole_radius_)))
	{
		pos.z = -Table::bound_z_;
	}
	else
	{
		// simple clamp if not in corner range
		pos.x = glm::clamp(pos.x, -Table::bound_x_, Table::bound_x_);
		pos.z = glm::clamp(pos.z, -Table::bound_z_, Table::bound_z_);
	}

	// 2) Additionally, block holes. If distance < Table::hole_radius_ + Ball::radius_, push it out
	for (auto& hCenter : table_->GetHoles())
	{
		glm::vec2 p2d(pos.x, pos.z);
		glm::vec2 h2d(hCenter.x, hCenter.z);

		float dist = glm::distance(p2d, h2d);
		float minDist = Table::hole_radius_ + Ball::radius_;

		if (dist < minDist)
		{
			// push pos out along direction from hole -> pos
			glm::vec2 dir2d = glm::normalize(p2d - h2d);
			glm::vec2 new2d = h2d + dir2d * minDist;
			pos.x = new2d.x;
			pos.z = new2d.y;
		}
	}

	return pos;
}

void World::PlaceCueBallWithMouse()
{
	GLFWwindow* window = glfwGetCurrentContext(); if (!window) return;


	double mouseX, mouseY; glfwGetCursorPos(window, &mouseX, &mouseY);
	int screenW, screenH; glfwGetWindowSize(window, &screenW, &screenH);


	auto [rayOrigin, rayDir] = camera_.GetMouseRay((float)mouseX, (float)mouseY, screenW, screenH);
	auto [hit, intersect] = RayIntersectTablePlane(rayOrigin, rayDir);
	if (!hit) return;


	glm::vec3 finalPos(intersect.x, Ball::radius_, intersect.z);
	finalPos = ClampCueBallPosition(finalPos);


	auto& cueBall = balls_[0];
	cueBall->TakeFromHole(); cueBall->SetDrawn(true); cueBall->Translate(finalPos);


	for (int i = 1; i < (int)balls_.size(); ++i) if (balls_[i]->IsDrawn()) {
		float d = glm::distance(cueBall->translation_, balls_[i]->translation_);
		if (d < 2.0f * Ball::radius_) {
			state_.SetBallInHand(false);
			state_.SetMessage("Foul! Illegal contact during ball-in-hand.", 1.2f);
			state_.SwitchTurn(balls_[0]->IsDrawn());
			state_.SetBallInHand(true);
			return;
		}
	}


	HandleBoundsCollision(0);
	cueBall->translation_.y = Ball::radius_;
	cue_->PlaceAtBall(cueBall);


	static bool wasPressed = false;
	int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (mouseState == GLFW_PRESS) {
		wasPressed = true;
	}
	else if (wasPressed) {
		wasPressed = false;
		state_.SetBallInHand(false);
		state_.ResetShotClock();
	}
}
