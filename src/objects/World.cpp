#include "../stdafx.h"
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
	ceiling_(std::make_shared<Ceiling>(Config::ceiling_path, glm::vec3(0.0f, 1.48f, 0.04f), glm::vec3(0.4f), glm::vec3(0.0f, 1.0f, 0.0f))),
	players_({ Player("Player 1"), Player("Player 2") })
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
	GLFWwindow* window = glfwGetCurrentContext();
	if (!window) return;

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	int screenW, screenH;
	glfwGetWindowSize(window, &screenW, &screenH);

	// 1) Ray from camera
	auto [rayOrigin, rayDir] = camera_.GetMouseRay((float)mouseX, (float)mouseY, screenW, screenH);

	// 2) Intersect plane y=0
	auto [hit, intersect] = RayIntersectTablePlane(rayOrigin, rayDir);
	if (!hit) return; // no intersection, do nothing

	// 3) We want ball center at (x, Ball::radius_, z)
	glm::vec3 finalPos(intersect.x, Ball::radius_, intersect.z);
	finalPos = ClampCueBallPosition(finalPos);

	// 4) Place the cue ball
	auto& cueBall = balls_[0];
	cueBall->TakeFromHole();
	cueBall->SetDrawn(true);

	cueBall->Translate(finalPos);

	// Check for illegal contact during ball-in-hand placement
	for (int i = 1; i < balls_.size(); ++i) {
		if (balls_[i]->IsDrawn()) {
			float distance = glm::distance(cueBall->translation_, balls_[i]->translation_);
			if (distance < 2.0f * Ball::radius_) {
				//Logger::Log("Foul: Cue ball hit another ball during ball-in-hand placement.");
				ball_in_hand_ = false;
				SetMessage("Foul! Illegal contact during ball-in-hand.", 1.2f);
				SwitchTurn();
				ball_in_hand_ = true;
				return; // Exit early due to foul
			}
		}
	}

	// 5) Reuse existing cushion/pocket logic
	HandleBoundsCollision(0);

	// 6) Force y=Ball::radius_ again after collision
	cueBall->translation_.y = Ball::radius_;

	// 7) Place the cue visually
	cue_->PlaceAtBall(cueBall);

	// 8) Finalize on mouse release (RMB or LMB, your choice)
	static bool wasPressed = false;
	int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

	if (mouseState == GLFW_PRESS) {
		wasPressed = true;
	}
	else {
		if (wasPressed) {
			wasPressed = false;
			ball_in_hand_ = false;
			shot_clock_ = SHOT_CLOCK_MAX_;
		}
	}
}


void World::HandleBallsCollision(const int number)
{
	for (int j = number + 1; j < balls_.size(); j++)
	{
		const auto& first_ball = balls_[number];
		const auto& second_ball = balls_[j];

		bool cueCollisionHappened = false;

		// Check if one of the balls is the cue ball (assumed ball 0)
		if (number == 0 || j == 0) {
			float distance = glm::length(first_ball->translation_ - second_ball->translation_);
			float collisionDistance = 2.0f * Ball::radius_;
			float epsilon = 0.0001f; // small buffer to account for precision

			if (distance <= (collisionDistance + epsilon)) {
				cueCollisionHappened = true;
			}
		}

		// Process the collision between the two balls
		first_ball->CollideWith(second_ball);

		// If a collision with the cue ball was detected, update flags and record first contact
		if (cueCollisionHappened && (number == 0 || j == 0)) {
			cueBallHitOtherBall_ = true;

			if (firstBallContactIndex_ == -1) {
				// Identify the non-cue ball as the first contact
				int objectIndex = (number == 0 ? j : number);
				if (objectIndex != 0)
					firstBallContactIndex_ = objectIndex;
			}
		}
	}
}


void World::HandleHolesFall(const int number) const
{
	if (!AreBallsInMotion())
	{
		// Cue ball pocketed => we’ll handle as foul in CheckGameRules
		if (number == 0)
		{
			balls_[number]->SetDrawn(false);
			return;
		}
		// Any other ball is removed from play
		balls_[number]->SetDrawn(false);
		return;
	}

	// If still moving, handle "falling into the hole" with gravity
	balls_[number]->HandleGravity(Table::hole_bottom_);

	const auto translation_horizontal = glm::vec2(balls_[number]->translation_.x, balls_[number]->translation_.z);
	const auto hole_horizontal = glm::vec2(balls_[number]->GetHole().x, balls_[number]->GetHole().z);

	const glm::vec2 direction = glm::normalize(translation_horizontal - hole_horizontal);
	const float distance = glm::distance(translation_horizontal, hole_horizontal);

	if (distance > Table::hole_radius_ - Ball::radius_)
		balls_[number]->BounceOffHole(-direction, Table::hole_radius_);
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

void World::Update(float dt, bool in_game)
{
	if (message_timer_ > 0.f)
	{
		message_timer_ -= dt;
		if (message_timer_ <= 0.f)
		{
			current_message_.clear();
			message_timer_ = 0.f;
		}
	}

	if (is_game_over_) return;

	if (in_game)
	{
		// Shot clock
		shot_clock_ -= dt;
		if (shot_clock_ <= 0.f)
		{
			ball_in_hand_ = true;
			SetMessage("Foul! Shot clock expired.", 1.2f);
			SwitchTurn();
		}

		if (ball_in_hand_) {
			PlaceCueBallWithMouse();
		}
		else {
			cue_->HandleShot(balls_[0], dt);
		}

	}

	static bool wasMoving = false;
	bool nowMoving = AreBallsInMotion();
	if (!wasMoving && nowMoving)
	{
		// new shot started
		firstBallContactIndex_ = -1;
		cueBallHitOtherBall_ = false;

	}
	wasMoving = nowMoving;


	// Move each ball
	for (int i = 0; i < (int)balls_.size(); i++)
	{
		balls_[i]->Roll(dt);

		if (balls_[i]->IsInHole(table_->GetHoles(), Table::hole_radius_))
			HandleHolesFall(i);
		else
			HandleBoundsCollision(i);

		// Force the ball center to y=Ball::radius_ if it’s on table
		if (!balls_[i]->IsInHole(table_->GetHoles(), Table::hole_radius_))
		{
			balls_[i]->translation_.y = Ball::radius_;
		}

		HandleBallsCollision(i);
	}

	// If everything is still => we apply CheckGameRules() *once*
	if (!AreBallsInMotion())
	{
		if (check_game_rules_)
		{
			CheckGameRules();
			check_game_rules_ = false;
		}
	}
	else
	{
		check_game_rules_ = true;
	}
}

bool World::AreBallsInMotion() const
{
	for (const auto& ball : balls_)
		if (ball->IsInMotion())
			return true;

	return false;
}

void World::Init() const
{
    // setting the white ball and cue in the initial position
	balls_[0]->Translate(glm::vec3(0.8f, 0.0f, 0.0f));
	cue_->translation_ = glm::vec3(0.0f);
	cue_->angle_ = 0.0f;
	cue_->Translate(glm::vec3(0.8f + Ball::radius_ + Config::min_change, Ball::radius_, 0.0f));
	cue_->Rotate(glm::vec3(-0.1f, 1.0f, 0.0f), glm::pi<float>());

    // creating a triangle
	balls_[1]->Translate(glm::vec3(-0.8f + 2.0f * glm::root_three<float>() * Ball::radius_, 0.0f, 0.0f));

	glm::vec3 temp = balls_[1]->translation_;
	int index = 2;
	for (int i = 0; i < 4; i++)
	{
		temp.x -= glm::root_three<float>() * Ball::radius_;
		temp.z -= Ball::radius_;
		for (int j = 0; j < i + 2; j++)
		{
			balls_[index]->Translate(glm::vec3(temp.x, 0.0f, temp.z + j * (Ball::radius_ * 2.0f)));
			index++;
		}
	}
}

void World::Reset() const
{
	for (const auto& ball : balls_)
	{
		ball->TakeFromHole();
		ball->SetDrawn(true);
	}

	Init();
}

void World::SwitchTurn()
{
	current_player_index_ = (current_player_index_ + 1) % players_.size();
	shot_clock_ = SHOT_CLOCK_MAX_;

	// If the cue ball is not drawn => it was pocketed => ball in hand
	if (!balls_[0]->IsDrawn())
	{
		ball_in_hand_ = true;
	}
}

bool World::AreAllGroupBallsPocketed(int groupType) const
{
	// groupType = 0 => solids (1..7), groupType = 1 => stripes (9..15)
	// Return true if all those balls are no longer drawn (i.e. pocketed)
	if (groupType == -1)
		return false; // Groups not assigned yet

	if (groupType == 0) // solids
	{
		for (int i = 1; i <= 7; ++i)
		{
			if (balls_[i]->IsDrawn()) // not pocketed
				return false;
		}
		return true;
	}
	else // groupType == 1 => stripes
	{
		for (int i = 9; i <= 15; ++i)
		{
			if (balls_[i]->IsDrawn())
				return false;
		}
		return true;
	}
}

void World::CheckGameRules()
{
	//Logger::Log("=== Entering CheckGameRules() ===");

	// 1) If the game is already over, skip
	if (is_game_over_)
	{
		//Logger::Log("CheckGameRules: Game is already over, returning.");
		return;
	}

	bool foul = false;
	bool ballPocketed = false;
	bool eightBallPocketed = false;
	bool playerContinues = false;

	// 0) If both players have assigned groups => definitely not open
	if (player1_ball_type_ != -1 && player2_ball_type_ != -1)
	{
		//Logger::Log("Both players have assigned groups => setting is_first_shot_ = false.");
		is_first_shot_ = false;
	}

	//------------------------------------------------------
	// 1) Cue ball pocketed => foul
	//------------------------------------------------------
	if (!balls_[0]->IsDrawn())  // means cue ball is no longer visible => pocketed
	{
		//Logger::Log("Cue ball is not drawn => pocketed => foul.");
		foul = true;
		ball_in_hand_ = true;
	}

	//------------------------------------------------------
	// 2) If no contact => foul
	//------------------------------------------------------
	if (!cueBallHitOtherBall_ && !ball_in_hand_)
	{
		//Logger::Log("No contact with another ball => foul.");
		foul = true;
		ball_in_hand_ = true;
	}

	//------------------------------------------------------
	// 3) If no foul so far, check “first ball contact” logic
	//------------------------------------------------------
	if (!foul)
	{
		// Table open if is_first_shot_ is still true
		bool tableOpen = is_first_shot_;

		if (tableOpen)
		{
			//Logger::Log("Table is open (is_first_shot_ == true).");
			// If table is open and firstBallContactIndex_ == 8 => foul
			if (firstBallContactIndex_ == 8 && firstBallContactIndex_ != -1)
			{
				//Logger::Log("Open table but first contact is 8 => foul.");
				foul = true;
				ball_in_hand_ = true;
			}
		}
		else
		{
			//Logger::Log("Table is NOT open (is_first_shot_ == false). Checking assigned groups...");
			// must contact your group first if assigned
			int curGroup = (current_player_index_ == 0)
				? player1_ball_type_
				: player2_ball_type_;

			if (curGroup != -1 && firstBallContactIndex_ != -1)
			{
				// If it’s the 8 ball first but group not cleared => foul
				if (firstBallContactIndex_ == 8 && !AreAllGroupBallsPocketed(curGroup))
				{
					//Logger::Log("Contacted the 8-ball first but group not cleared => foul.");
					foul = true;
					ball_in_hand_ = true;
				}
				else
				{
					// If it’s the opponent’s group => foul
					int contactType = (firstBallContactIndex_ < 8) ? 0 : 1;
					if (contactType != curGroup && firstBallContactIndex_ != 8)
					{
						//Logger::Log("Contacted opponent’s group first => foul.");
						foul = true;
						ball_in_hand_ = true;
					}
				}
			}
			else
			{
				//Logger::Log("No group assigned or no firstBallContactIndex => skipping group check.");
			}
		}
	}

	//------------------------------------------------------
	// 4) Detect potted balls
	//------------------------------------------------------
	int groupAssignmentBall = -1;
	//Logger::Log("Scanning for potted balls in 1..15...");

	for (int i = 1; i < 16; i++)
	{
		// if a ball i is no longer drawn => it's potted
		if (!balls_[i]->IsDrawn())
		{
			//Logger::Log("Ball " + std::to_string(i) + " is potted.");
			ballPocketed = true;

			if (i == 8)
			{
				eightBallPocketed = true;
			}
			else
			{
				// If table is open but NOT the break => we assign if we haven't assigned groups
				// or if you'd prefer to skip break entirely for assignment, check:
				// if (!is_after_break_ && is_first_shot_ && etc.)
				if (is_first_shot_ && !is_after_break_ &&
					(player1_ball_type_ == -1 || player2_ball_type_ == -1))
				{
					// We haven't assigned groups yet => pick the first potted ball 
					if (groupAssignmentBall == -1)
					{
						groupAssignmentBall = i;
						//Logger::Log("groupAssignmentBall set to " + std::to_string(i));
					}
				}

				int ballType = (i < 8) ? 0 : 1;
				int curGroup = (current_player_index_ == 0) ? player1_ball_type_ : player2_ball_type_;

				if (curGroup != -1)
				{
					// group assigned => check if it’s your group
					if (ballType == curGroup)
					{
						//Logger::Log("Potted a correct group ball => player continues.");
						playerContinues = true;
						players_[current_player_index_].AddScore(1);
					}
					else
					{
						//Logger::Log("Potted opponent’s group => foul.");
						foul = true;
						ball_in_hand_ = true;
					}
				}
				else
				{
					// table open => potting any non-8 => you keep shooting
					//Logger::Log("No group assigned => table open => potting non-8 => continue.");
					playerContinues = true;
				}
			}
		}
	}

	//------------------------------------------------------
	// 5) If we found a first potted ball => assign group
	//------------------------------------------------------
	if (groupAssignmentBall != -1)
	{
		//Logger::Log("About to call AssignBallType for ballNumber=" + std::to_string(groupAssignmentBall));
		AssignBallType(groupAssignmentBall);
		is_first_shot_ = false; // table no longer open
	}

	//------------------------------------------------------
	// 6) Check if the 8-ball was potted => see if it’s legal
	//------------------------------------------------------
	if (eightBallPocketed)
	{
		//Logger::Log("8-ball was potted. Checking if cleared or foul...");
		int curGroup = (current_player_index_ == 0) ? player1_ball_type_ : player2_ball_type_;
		bool cleared = AreAllGroupBallsPocketed(curGroup);

		if (!cleared || foul)
		{
			//Logger::Log("8-ball potted illegally => game over => loss.");
			is_game_over_ = true;
			SetMessage("Player " + players_[current_player_index_].GetName() + " pockets 8-ball and loses!", 2.f);
		}
		else
		{
			//Logger::Log("8-ball potted legally => game over => win!");
			is_game_over_ = true;
			SetMessage("Player " + players_[current_player_index_].GetName() + " wins by pocketing 8-ball!", 2.f);
		}
		return;
	}

	//------------------------------------------------------
	// 7) Handle post-break logic
	//------------------------------------------------------
	if (is_after_break_)
	{
		//Logger::Log("Post-break shot => not switching turn unless foul occurred.");
		// If we had a foul => we already switched turn
		// If not => the breaker continues
		is_after_break_ = false;
		//Logger::Log("Exiting CheckGameRules after break logic.");
		return;
	}

	//------------------------------------------------------
	// 8) If foul => next turn
	//------------------------------------------------------
	if (foul)
	{
		//Logger::Log("CheckGameRules => foul => switching turn => ball in hand for opponent.");
		SetMessage("Foul! Ball in hand for opponent.", 1.2f);
		SwitchTurn();
	}
	else
	{
		// no foul => see if we continue or switch
		if (!playerContinues)
		{
			//Logger::Log("No ball from your group was potted => switching turn.");
			SwitchTurn();
		}
		else
		{
			//Logger::Log("Player continues => resetting shot clock.");
			shot_clock_ = SHOT_CLOCK_MAX_;
		}
	}

	//Logger::Log("=== Exiting CheckGameRules() normally ===");
}


void World::AssignBallType(int ballNumber)
{
	// Debug info
	//Logger::Log("AssignBallType called with ballNumber=" + std::to_string(ballNumber));

	// Determine if this ball is “solids” (1..7) or “stripes” (9..15)
	bool isSolids = (ballNumber >= 1 && ballNumber <= 7);
	bool isStripes = (ballNumber >= 9 && ballNumber <= 15);

	if (isSolids)
	{
		// current player => solids (0)
		if (current_player_index_ == 0)
		{
			player1_ball_type_ = 0;  // 0 => solids
			player2_ball_type_ = 1;  // 1 => stripes
		}
		else
		{
			player2_ball_type_ = 0;
			player1_ball_type_ = 1;
		}
	}
	else if (isStripes)
	{
		// current player => stripes (1)
		if (current_player_index_ == 0)
		{
			player1_ball_type_ = 1;
			player2_ball_type_ = 0;
		}
		else
		{
			player2_ball_type_ = 1;
			player1_ball_type_ = 0;
		}
	}
	else
	{
		// If it's neither 1..7 nor 9..15, just log or ignore
		//Logger::Log("Warning: AssignBallType called but ballNumber out of range. No assignment done.");
		return;
	}

	// As soon as we do this assignment, the table is no longer open
	is_first_shot_ = false;

	//// Print debug for clarity
	//Logger::Log("Groups assigned => Player1: " +
	//	std::string((player1_ball_type_ == 0) ? "solids" : "stripes") +
	//	", Player2: " +
	//	std::string((player2_ball_type_ == 0) ? "solids" : "stripes"));
}


/** Helper to show a message on screen for 'duration' seconds */
void World::SetMessage(const std::string& msg, float duration)
{
	current_message_ = msg;
	message_timer_ = duration;
}

void World::ResetPlayerIndex()
{
	current_player_index_ = 0;
}

void World::ResetGame() 
{
	for (auto& player : players_) {
		player.ResetScore();
	}

	shot_clock_ = SHOT_CLOCK_MAX_;

	current_player_index_ = 0;
	
}
