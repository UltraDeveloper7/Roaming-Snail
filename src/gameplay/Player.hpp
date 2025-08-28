#pragma once
#include "../precompiled.h"

class Player {
public:
    explicit Player(const std::string& name) : name_(name), score_(0) {}

    void AddScore(int points) { score_ += points; }
    void ResetScore() { score_ = 0; }
    [[nodiscard]] int GetScore() const { return score_; }
    [[nodiscard]] const std::string& GetName() const { return name_; }

private:
    std::string name_;
    int score_;
};
