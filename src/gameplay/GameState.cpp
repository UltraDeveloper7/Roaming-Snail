#include "../stdafx.h"
#include "GameState.hpp"


GameState::GameState() = default;


void GameState::ResetPlayersScores() {
	for (auto& p : players_) p.ResetScore();
}


void GameState::ResetPlayerIndex() { current_player_index_ = 0; }


void GameState::StartNewRack() {
	is_game_over_ = false;
	is_first_shot_ = true;
	is_after_break_ = true;
	check_game_rules_ = false;
	ball_in_hand_ = false;
	shot_clock_ = SHOT_CLOCK_MAX;
	ClearShotTransients();
}


void GameState::StartNewTurn() {
	ClearShotTransients();
}


void GameState::SwitchTurn(bool cueBallDrawn) {
	current_player_index_ = (current_player_index_ + 1) % players_.size();
	shot_clock_ = SHOT_CLOCK_MAX;

	// If BIH was already granted (e.g., foul), KEEP it.
	// Otherwise, grant BIH only when the cue ball was pocketed.
	if (!ball_in_hand_) {
		ball_in_hand_ = !cueBallDrawn;
	}
}


void GameState::ResetShotClock() { shot_clock_ = SHOT_CLOCK_MAX; }


bool GameState::TickShotClock(float dt) {
	shot_clock_ -= dt;
	if (shot_clock_ <= 0.f) {
		shot_clock_ = 0.f;
		ball_in_hand_ = true;
		return true; // expired now
	}
	return false;
}


void GameState::TickMessage(float dt) {
	if (message_timer_ > 0.f) {
		message_timer_ -= dt;
		if (message_timer_ <= 0.f) {
			current_message_.clear();
			message_timer_ = 0.f;
		}
	}
}


void GameState::MarkCueContact(int nonCueBallIndex) {
	cueBallHitOtherBall_ = true;
	if (firstBallContactIndex_ == -1 && nonCueBallIndex != 0) {
		firstBallContactIndex_ = nonCueBallIndex;
	}
}


void GameState::ClearShotTransients() {
	cueBallHitOtherBall_ = false;
	firstBallContactIndex_ = -1;
}


void GameState::SetMessage(const std::string& msg, float seconds) {
	current_message_ = msg;
	message_timer_ = seconds;
}


void GameState::AssignGroupFromBallNumber(int ballNumber) {
	const bool solids = (ballNumber >= 1 && ballNumber <= 7);
	const bool stripes = (ballNumber >= 9 && ballNumber <= 15);
	if (!solids && !stripes) return;


	if (current_player_index_ == 0) {
		player1_ball_type_ = solids ? 0 : 1;
		player2_ball_type_ = solids ? 1 : 0;
	}
	else {
		player2_ball_type_ = solids ? 0 : 1;
		player1_ball_type_ = solids ? 1 : 0;
	}
	is_first_shot_ = false;
}