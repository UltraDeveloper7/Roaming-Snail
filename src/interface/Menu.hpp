#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <format>
#include <utility> // For std::forward

template <typename... Args>
std::string BuildString(std::string_view fmt, Args&&... args) {
	// Pass arguments directly; do NOT wrap in a tuple.
	return std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
}

enum class Alignment
{
	LEFT,
	CENTER,
	RIGHT
};

struct Text
{
	float u{};
	float v{};
	std::string text{};
	float scale{ 1.0f };
	Alignment alignment{};
	bool selected{ false };
};

class Menu
{
public:
	Menu(int width, int height);

	[[nodiscard]] std::vector<Text>& GetTexts() { return texts_; }
	[[nodiscard]] int GetSelected() const { return selected_; }

	void Draw(bool not_loaded, bool has_started);
	void Update(int width, int height);
	void AddText(float u, float v, const std::string& text, float scale = 1.0f, Alignment alignment = Alignment::LEFT, bool selected = false);


	// click events (edge-triggered: true once per frame when clicked)
	bool ConsumePlayClicked();
	bool ConsumeExitClicked();
	bool ConsumeResetClicked();

	// settings dropdown state (read-only)
	bool IsSettingsOpen() const { return settings_open_; }
	bool IsHelpOpen()     const { return help_open_; }


private:
	void ControlState();
	void beginFrameCaptureMouse();
	bool button(float u, float v, const std::string& label, float scale,
		Alignment align, bool emphasize, float* out_w = nullptr, float* out_h = nullptr);

	// very rough text metrics
	float estimateWidthPx(const std::string& s, float scale) const;
	float estimateHeightPx(float scale) const;

	int width_{};
	int height_{};
	int selected_{};
	int last_left_state_{}, last_up_state_{}, last_right_state_{}, last_down_state_{};
	std::vector<Text> texts_{};

	// mouse
	double mouse_x_{ 0.0 }, mouse_y_{ 0.0 }; // bottom-left origin
	int last_mouse_{ 0 }; // GLFW_RELEASE / PRESS

	// UI state
	bool help_open_{ false };
	bool settings_open_{ false };

	// click latches
	bool play_clicked_{ false };
	bool exit_clicked_{ false };
	bool reset_clicked_{ false };
	bool mouse_edge_down_ = false;
};