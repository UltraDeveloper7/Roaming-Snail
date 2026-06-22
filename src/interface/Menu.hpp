#pragma once

#include "../precompiled.h"

enum class MenuScreen
{
	Main,
	Pause
};

// Actions requested by the menu. App consumes these and changes game state.
enum class MenuAction
{
	None,
	Play,
	Resume,
	Reset,
	Settings,
	Help,
	Exit
};

enum class Alignment
{
	LEFT,
	CENTER,
	RIGHT
};

struct Text
{
	// Pixel-space position generated from normalized UI coordinates.
	float position_x = 0.0f;
	float position_y = 0.0f;
	std::string text;
	// TextRenderer scale multiplier.
	float scale = 1.0f;
	Alignment alignment = Alignment::LEFT;
	// Selected items are highlighted by TextRenderer.
	bool selected = false;
};

class Menu final
{
public:
	Menu(int width, int height);

	// Updates layout and input state for the current window size.
	void Update(int width, int height);
	// Rebuilds the text command list for main/pause/modals.
	void Draw(bool hasStarted, bool paused);

	// Returns the latest action and resets it to None.
	MenuAction ConsumeAction();

	const std::vector<Text>& GetTexts() const { return texts_; }
	std::vector<Text>& GetTextsMutable() { return texts_; }

	bool IsAnyModalOpen() const;

private:
	// Clears per-frame text and mouse-edge state.
	void BeginFrame();
	void DrawMain();
	void DrawPause();
	void DrawSettingsModal();
	void DrawHelpModal();

	void AddText(
		float u,
		float v,
		const std::string& text,
		float scale,
		Alignment alignment = Alignment::CENTER,
		bool selected = false
	);

	// Adds a selectable text item and handles keyboard/mouse activation.
	bool Button(
		float u,
		float v,
		const std::string& label,
		float scale,
		Alignment alignment,
		bool selected
	);

	float EstimateWidthPx(const std::string& s, float scale) const;
	float EstimateHeightPx(float scale) const;

	// Converts design scale into actual scale after resolution-dependent UI size.
	float Ui(float scale) const { return scale * ui_scale_; }

private:
	// Current framebuffer size.
	int width_ = 0;
	int height_ = 0;

	// Mouse position in screen pixels.
	double mouse_x_ = 0.0;
	double mouse_y_ = 0.0;

	// True only on the frame the mouse button is pressed.
	bool mouse_edge_down_ = false;

	// Keyboard selection state for menu navigation.
	int selected_ = 0;
	int last_up_state_ = GLFW_RELEASE;
	int last_down_state_ = GLFW_RELEASE;
	int last_enter_state_ = GLFW_RELEASE;
	int last_escape_state_ = GLFW_RELEASE;

	bool settings_open_ = false;
	bool help_open_ = false;

	// Simple modal animation values.
	float settings_anim_ = 0.0f;
	float help_anim_ = 0.0f;

	// Responsive UI scale and toggles displayed in settings/help.
	float ui_scale_ = 1.0f;
	bool show_controls_hint_ = true;
	bool shadows_enabled_ui_ = true;

	MenuAction pending_action_ = MenuAction::None;

	// Text commands consumed by TextRenderer each frame.
	std::vector<Text> texts_;
};
