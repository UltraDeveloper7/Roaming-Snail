#pragma once
#include "../precompiled.h"

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

	void InstallCharCallback(GLFWwindow* window);

	// Read current names (for initial sync when the world is first created)
	const std::string& P1Name() const { return p1_name_; }
	const std::string& P2Name() const { return p2_name_; }

	// One-shot “consume if changed” (prevents spamming updates every frame)
	bool ConsumeEditedNames(std::string& outP1, std::string& outP2);


	// click events (edge-triggered: true once per frame when clicked)
	bool ConsumePlayClicked();
	bool ConsumeExitClicked();
	bool ConsumeResetClicked();

	// settings dropdown state (read-only)
	bool IsSettingsOpen() const { return settings_open_ || ui_settings_open_;
	}
	bool IsHelpOpen()     const { return help_open_; }

	// guideline setting, read-only from outside
	bool IsGuidelineOn() const { return show_guideline_; }


private:

	// ---------- UI scale + names ----------
	float ui_scale_ = 1.0f;                 // global UI multiplier
	static constexpr float kUiMin_ = 0.50f;
	static constexpr float kUiMax_ = 1.25f;

	bool names_dirty_ = false; // set true whenever the user edits P1/P2
	std::string p1_name_ = "Player 1";
	std::string p2_name_ = "Player 2";


	int  active_input_ = -1;                // -1 none, 0=P1, 1=P2
	bool ui_settings_open_ = false;         // NEW: central Settings modal
	bool rename_gate_open_ = false;         // set true when "Reset game" clicked (pause)

	// ---------- helpers ----------
	void DrawMainMenu(bool modalOpen, int winW, int winH,
		float mouseX, float mouseY, int curEnter,
		int& prevEnter, int& selected);
	void DrawPauseMenu(bool modalOpen, int winW, int winH,
		float mouseX, float mouseY, int curEnter,
		int& prevEnter, int& selected);
	void DrawQuickSetupModal(int winW, int winH, float mouseX, float mouseY, bool has_started);
	void DrawHelpModal(bool has_started);
	bool DrawSettingsIcon(int winW, int winH, bool selected);         // bottom-right launcher
	void DrawUiSettingsModal(bool has_started);        // centered modal

	inline float Ui(float s) const { return s * ui_scale_; }

	void ControlState();
	void beginFrameCaptureMouse();
	bool button(float u, float v, const std::string& label, float scale,
		Alignment align, bool emphasize, float* out_w = nullptr, float* out_h = nullptr);

	// very rough text metrics
	float estimateWidthPx(const std::string& s, float scale) const;
	float estimateHeightPx(float scale) const;

	static void CharCallbackThunk(GLFWwindow* wnd, unsigned int codepoint);
	static std::u32string char_buffer_;  // per-frame typed Unicode points

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
	bool show_guideline_ = true; // default on
};