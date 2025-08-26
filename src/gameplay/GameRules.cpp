#include "../stdafx.h"
#include "GameRules.hpp"
#include "GameState.hpp"
#include "../objects/Ball.hpp"


static int BallTypeFromIndex(int i) { return (i < 8) ? 0 : 1; } // 0 solids, 1 stripes


bool GameRules::AreAllGroupBallsPocketed(const std::vector<std::shared_ptr<Ball>>& balls, int groupType)
{
	if (groupType == -1) return false;
	if (groupType == 0) {
		for (int i = 1; i <= 7; ++i) if (balls[i]->IsDrawn()) return false; return true;
	}
	else {
		for (int i = 9; i <= 15; ++i) if (balls[i]->IsDrawn()) return false; return true;
	}
}


void GameRules::EvaluateEndOfShot(const std::vector<std::shared_ptr<Ball>>& balls, GameState& s)
{
	if (s.IsGameOver()) return;


	bool foul = false;
	bool ballPocketed = false;
	bool eightBallPocketed = false;
	bool playerContinues = false;


	// Once both groups fixed => table not open
	if (s.GroupOfPlayer(0) != -1 && s.GroupOfPlayer(1) != -1) {
		s.SetFirstShot(false);
	}


	// 1) Cue ball pocketed => foul & BIH
	if (!balls[0]->IsDrawn()) {
		foul = true;
		s.SetBallInHand(true);
	}


	// 2) No contact => foul (unless we are already in BIH placement)
	if (!s.CueHitOtherBall() && !s.BallInHand()) {
		foul = true;
		s.SetBallInHand(true);
	}


	// 3) First-contact rule
	if (!foul) {
		const bool tableOpen = s.IsFirstShot();
		const int firstIdx = s.FirstContactIndex();


		if (tableOpen) {
			if (firstIdx == 8 && firstIdx != -1) {
				foul = true; s.SetBallInHand(true);
			}
		}
		else {
			const int curGroup = s.CurrentPlayerGroup();
			if (curGroup != -1 && firstIdx != -1) {
				if (firstIdx == 8 && !AreAllGroupBallsPocketed(balls, curGroup)) {
					foul = true; s.SetBallInHand(true);
				}
				else {
					int contactType = BallTypeFromIndex(firstIdx);
					if (contactType != curGroup && firstIdx != 8) {
						foul = true; s.SetBallInHand(true);
					}
				}
			}
		}
	}


	// 4) Scan potted balls for scoring & potential group assignment
	int groupAssignmentBall = -1;
	for (int i = 1; i < 16; ++i) {
		if (!balls[i]->IsDrawn()) {
			ballPocketed = true;
			if (i == 8) {
				eightBallPocketed = true;
			}
			else {
				if (s.IsFirstShot() && !s.IsAfterBreak() &&
					(s.GroupOfPlayer(0) == -1 || s.GroupOfPlayer(1) == -1)) {
					if (groupAssignmentBall == -1) groupAssignmentBall = i;
				}


				const int ballType = BallTypeFromIndex(i);
				const int curGroup = s.CurrentPlayerGroup();
				if (curGroup != -1) {
					if (ballType == curGroup) {
						playerContinues = true;
						auto& p = s.Players()[s.CurrentPlayerIndex()];
						p.AddScore(1);
					}
					else {
						foul = true; s.SetBallInHand(true);
					}
				}
				else {
					// Table open
					playerContinues = true;
				}
			}
		}
	}


	// 5) Assign groups if needed
	if (groupAssignmentBall != -1) {
		s.AssignGroupFromBallNumber(groupAssignmentBall);
		s.SetFirstShot(false);
	}


	// 6) 8-ball outcome
	if (eightBallPocketed) {
		const int curGroup = s.CurrentPlayerGroup();
		const bool cleared = AreAllGroupBallsPocketed(balls, curGroup);
		if (!cleared || foul) {
			s.SetGameOver(true);
			s.SetMessage("Player " + s.Players()[s.CurrentPlayerIndex()].GetName() + " pockets 8-ball and loses!", 2.f);
		}
		else {
			s.SetGameOver(true);
			s.SetMessage("Player " + s.Players()[s.CurrentPlayerIndex()].GetName() + " wins by pocketing 8-ball!", 2.f);
		}
		return;
	}

	// 7) Post-break: breaker keeps turn unless there was a foul above
	if (s.IsAfterBreak()) {
		s.SetAfterBreak(false);

		if (foul) {
			s.SetMessage("Foul! Ball in hand for opponent.", 1.2f);
			s.SetBallInHand(true);                      // <-- grant BIH on foul
			const bool cueDrawn = balls[0]->IsDrawn();
			s.SwitchTurn(cueDrawn);
		}
		return; // done handling post-break resolution
	}

	// 8) Final turn decision for normal shots
	if (foul) {
		s.SetMessage("Foul! Ball in hand for opponent.", 1.2f);
		s.SetBallInHand(true);                          // <-- grant BIH on foul
		const bool cueDrawn = balls[0]->IsDrawn();
		s.SwitchTurn(cueDrawn);
	}
	else {
		if (!playerContinues) {
			const bool cueDrawn = balls[0]->IsDrawn();
			s.SwitchTurn(cueDrawn);
		}
		else {
			s.ResetShotClock();
		}
	}
}