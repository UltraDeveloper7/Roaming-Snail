#include "../precompiled.h"
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

    const bool onBreak = s.IsAfterBreak();
    const bool tableOpen = s.IsFirstShot();

    // Once both groups fixed => table not open
    if (s.GroupOfPlayer(0) != -1 && s.GroupOfPlayer(1) != -1) {
        s.SetFirstShot(false);
    }

    // 1) Cue ball pocketed => foul & BIH
    if (!balls[0]->IsDrawn()) {
        foul = true;
        s.SetBallInHand(true);
    }

    // 2) No contact => foul (unless already placing BIH)
    if (!s.CueHitOtherBall() && !s.BallInHand()) {
        foul = true;
        s.SetBallInHand(true);
    }

    // 3) First-contact rule
    // - On the break, ANY first contact is okay (including the 8).
    // - On the open table after the break (called-shot play), first contact must not be the 8.
    // - Once groups are assigned, first contact must be your group unless you’re legally on the 8.
    if (!foul) {
        const int firstIdx = s.FirstContactIndex();

        if (tableOpen && !onBreak) {
            // open table, but not the break → cannot first-contact the 8
            if (firstIdx == 8 && firstIdx != -1) {
                foul = true; s.SetBallInHand(true);
            }
        }
        else if (!tableOpen) {
            const int curGroup = s.CurrentPlayerGroup();
            if (curGroup != -1 && firstIdx != -1) {
                // If you're not on the 8 yet, first contact must be your group
                if (firstIdx == 8 && !AreAllGroupBallsPocketed(balls, curGroup)) {
                    foul = true; s.SetBallInHand(true);
                }
                else if (firstIdx != 8) {
                    int contactType = BallTypeFromIndex(firstIdx);
                    if (contactType != curGroup) {
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
                const int ballType = BallTypeFromIndex(i);

                // Assign groups on the first legal, called (post-break) pot when table is open
                if (tableOpen && !onBreak &&
                    (s.GroupOfPlayer(0) == -1 || s.GroupOfPlayer(1) == -1)) {
                    if (groupAssignmentBall == -1) groupAssignmentBall = i;
                }

                // Scoring / continuation:
                const int curGroup = s.CurrentPlayerGroup();
                if (curGroup != -1) {
                    if (ballType == curGroup) {
                        playerContinues = true;
                        s.Players()[s.CurrentPlayerIndex()].AddScore(1);
                    }
                    // Pocketing an opponent ball is NOT a foul by itself; you just don't continue.
                }
                else {
                    // Table open (post-break, called play) — pocketing any non-8 assigns later and continues
                    playerContinues = true;
                }
            }
        }
    }

    // 5) Cushion-after-contact rule:
    // If no ball was pocketed this shot, at least one ball must have hit a rail after legal contact.
    if (!ballPocketed && !s.RailAfterContact()) {
        foul = true;
        s.SetBallInHand(true);
    }

    // 6) 8-ball outcomes
    // Special case: 8-ball on the break
    if (eightBallPocketed && onBreak) {
        if (foul) {
            s.SetGameOver(true);
            s.SetMessage("Scratch with 8-ball on the break — loss.", 2.f);
        }
        else {
            s.SetGameOver(true);
            s.SetMessage("8-ball on the break — WIN!", 2.f);
        }
        return;
    }

    // Normal (non-break) 8-ball pocket
    if (eightBallPocketed) {
        const int curGroup = s.CurrentPlayerGroup();
        const bool cleared = AreAllGroupBallsPocketed(balls, curGroup);
        if (!cleared || foul) {
            s.SetGameOver(true);
            s.SetMessage("8-ball pocketed illegally — loss.", 2.f);
        }
        else {
            s.SetGameOver(true);
            s.SetMessage("8-ball pocketed — WIN!", 2.f);
        }
        return;
    }

    // 7) Post-break resolution (no 8-ball fell)
    if (onBreak) {
        s.SetAfterBreak(false);

        if (foul) {
            s.SetMessage("Foul on the break. Ball in hand for opponent.", 1.2f);
            const bool cueDrawn = balls[0]->IsDrawn();
            s.SwitchTurn(cueDrawn);
        }
        else {
            if (ballPocketed) {
                // breaker keeps the table
                s.ResetShotClock();
                // groups remain open; assignment happens on the first called make later
            }
            else {
                // dry break → opponent shoots
                const bool cueDrawn = balls[0]->IsDrawn();
                s.SwitchTurn(cueDrawn);
            }
        }
        return;
    }

    // 8) Normal turn resolution
    if (foul) {
        s.SetMessage("Foul! Ball in hand for opponent.", 1.2f);
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

    // 9) Apply any pending group assignment (first legally pocketed non-8 on open table post-break)
    if (groupAssignmentBall != -1) {
        s.AssignGroupFromBallNumber(groupAssignmentBall);
        s.SetFirstShot(false);
    }
}
