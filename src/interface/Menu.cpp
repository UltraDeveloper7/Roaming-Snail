#include "../stdafx.h"
#include "Menu.hpp"

namespace {
    // One source of truth for the help lines
    static const char* kHelpLines[] = {
        "Edit options with LEFT/RIGHT",
        "Move camera: W S A D E Q",
        "Rotate camera: mouse",
        "Adjust power: UP/DOWN",
        "Rotate cue: LEFT/RIGHT",
        "Strike: SPACE",
        "Toggle lights: keys 0-9",
    };
    constexpr int kHelpCount = static_cast<int>(sizeof(kHelpLines) / sizeof(kHelpLines[0]));

    // Generic renderer for the help text; draws at any anchor/scale/alignment
    static void RenderHelpBlock(
        Menu& menu, float uAnchor, float vTop,
        Alignment align, float scale, float lineGap)
    {
        float v = vTop;
        for (int i = 0; i < kHelpCount; ++i) {
            menu.AddText(uAnchor, v, kHelpLines[i], scale, align, false);
            v -= lineGap;
        }
    }
} 

// --------------------------------------------------------------------//

Menu::Menu(const int width, const int height) : width_(width), height_(height) {
    // no initial selection → nothing is red until hover/arrow keys
    selected_ = -1;
}

void Menu::Update(const int width, const int height) { width_ = width; height_ = height; }

void Menu::beginFrameCaptureMouse()
{
    texts_.clear();
    GLFWwindow* w = glfwGetCurrentContext();
    double mx, my;
    glfwGetCursorPos(w, &mx, &my);
    // convert to bottom-left origin to match our text coordinates
    int ww, wh;
    glfwGetWindowSize(w, &ww, &wh);
    mouse_x_ = mx;
    mouse_y_ = wh - my;
}

float Menu::estimateWidthPx(const std::string& s, float scale) const
{
    // very rough: average glyph ≈ 0.62 * font_size
    constexpr float kAvg = 0.62f;
    const float base = static_cast<float>(Config::default_font_size);
    return static_cast<float>(s.size()) * base * kAvg * scale;
}
float Menu::estimateHeightPx(float scale) const
{
    const float base = static_cast<float>(Config::default_font_size);
    return base * 1.1f * scale;
}

bool Menu::button(float u, float v, const std::string& label, float scale,
    Alignment align, bool emphasize, float* out_w, float* out_h)
{
    // compute rect
    float px = u * width_;
    float py = v * height_;
    float w = estimateWidthPx(label, scale);
    float h = estimateHeightPx(scale);

    float x0 = px, x1 = px + w;
    if (align == Alignment::CENTER) { x0 = px - w * 0.5f; x1 = px + w * 0.5f; }
    else if (align == Alignment::RIGHT) { x0 = px - w; x1 = px; }

    float y0 = py - h * 0.5f;
    float y1 = py + h * 0.5f;

    // hover?
    bool hover = (mouse_x_ >= x0 && mouse_x_ <= x1 && mouse_y_ >= y0 && mouse_y_ <= y1);

    // draw
    AddText(u, v, label, scale, align, emphasize || hover);

    // click edge
    GLFWwindow* wWin = glfwGetCurrentContext();
    int btn = glfwGetMouseButton(wWin, GLFW_MOUSE_BUTTON_LEFT);
    bool clicked = hover && (btn == GLFW_PRESS) && (last_mouse_ == GLFW_RELEASE);
    last_mouse_ = btn;

    if (out_w) *out_w = (x1 - x0);
    if (out_h) *out_h = (y1 - y0);
    return clicked;
}

void Menu::Draw(const bool not_loaded, const bool has_started)
{
    texts_.clear();
    GLFWwindow* w = glfwGetCurrentContext();

    const int winW = width_, winH = height_;
    double mx, my; glfwGetCursorPos(w, &mx, &my);
    const float mouseX = static_cast<float>(mx);
    const float mouseY = static_cast<float>(winH - my);

    static int prevMouse = GLFW_RELEASE;
    const int curMouse = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT);

    static int prevEnter = GLFW_RELEASE;
    const int curEnter = glfwGetKey(w, GLFW_KEY_ENTER);

    const bool modalOpen = settings_open_ || help_open_;

    // Normal keyboard navigation only when no modal is open
    if (!modalOpen) {
        ControlState();
    }

    // Selection order:
    //  - not started: 0=Play, 1=Initial conditions, 2=Help
    //  - started (pause menu center list): 0=Resume, 1=Reset, 2=Initial conditions, 3=Help, 4=Exit
    std::vector<int> items;
    if (!has_started) { items = { 0, 1, 2 }; }
    else { items = { 0, 1, 2, 3, 4 }; }

    if (selected_ != -1) {
        if (selected_ < 0) selected_ = static_cast<int>(items.size()) - 1;
        if (selected_ >= static_cast<int>(items.size())) selected_ = 0;
    }

    auto drawButton = [&](float u, float v, const std::string& label, float scale,
        Alignment align, bool emphasize, bool isSelected) -> bool
        {
            const float px = u * winW;
            const float py = v * winH;
            const float wpx = estimateWidthPx(label, scale);
            const float hpx = estimateHeightPx(scale);

            float x0, x1;
            if (align == Alignment::CENTER) { x0 = px - wpx * 0.5f; x1 = px + wpx * 0.5f; }
            else if (align == Alignment::RIGHT) { x0 = px - wpx; x1 = px; }
            else { x0 = px; x1 = px + wpx; }
            const float y0 = py - hpx * 0.5f;
            const float y1 = py + hpx * 0.5f;

            const bool hover = (mouseX >= x0 && mouseX <= x1 && mouseY >= y0 && mouseY <= y1);
            AddText(u, v, label, scale, align, emphasize || hover || isSelected);

            const bool clicked = hover && (curMouse == GLFW_PRESS && prevMouse == GLFW_RELEASE);
            return clicked;
        };

    auto isEnterOn = [&](int logicalIndex)->bool {
        if (selected_ == -1) return false;
        const bool pressed = (curEnter == GLFW_PRESS && prevEnter == GLFW_RELEASE);
        return pressed && (selected_ == logicalIndex);
        };

    // Title
    AddText(0.5f, 0.90f, "BILLIARDS", 2.0f, Alignment::CENTER);

    // ==============================================================
    //   NO GAME STARTED — initial menu layout
    // ==============================================================
    if (!has_started) {
        // Play
        const bool playSelected = (selected_ == 0);
        const bool playClicked = !modalOpen && (
            drawButton(0.5f, 0.70f, "Play game", 1.05f, Alignment::CENTER, false, playSelected) || isEnterOn(0)
            );
        if (playClicked) play_clicked_ = true;

        // Initial conditions (with > / v symbol)
        if (!settings_open_) {
            const bool condSelected = (selected_ == 1);
            const char* dd = settings_open_ ? "v" : ">";
            const bool condClicked = !modalOpen && drawButton(
                0.10f, 0.18f, std::string(dd) + "  Initial conditions", 0.85f,
                Alignment::LEFT, false, condSelected);
            if (condClicked || isEnterOn(1)) {
                settings_open_ = true;
                selected_ = 1;
            }
        }

        // Help (with [i] symbol)
        if (!help_open_) {
            const bool helpSelected = (selected_ == 2);
            const bool helpClicked = !modalOpen && drawButton(
                0.10f, 0.10f, "[i] Help", 0.90f,
                Alignment::LEFT, false, helpSelected);
            if (helpClicked || isEnterOn(2)) {
                help_open_ = true;
                selected_ = 2;
            }
        }
    }
    // ==============================================================
    //   GAME STARTED — pause menu in the CENTER
    // ==============================================================
    else {
        if (!modalOpen) {
            // Resume (kept at the same top-center place)
            const bool resumeSelected = (selected_ == 0);
            const bool resumeClicked = drawButton(0.5f, 0.84f,
                "Resume game", 1.05f, Alignment::CENTER, false, resumeSelected)
                || isEnterOn(0);
            if (resumeClicked) play_clicked_ = true;

            // Vertical list in the center (with symbols)
            const float startV = 0.62f;
            const float stepV = 0.08f;

            // 1) Reset
            const bool resetSelected = (selected_ == 1);
            if (drawButton(0.5f, startV + 0 * stepV, "Reset game", 0.90f,
                Alignment::CENTER, false, resetSelected) || isEnterOn(1)) {
                reset_clicked_ = true;
            }

            // 2) Initial conditions (show > or v accordingly)
            const bool condSelected = (selected_ == 2);
            const char* dd = settings_open_ ? "v" : ">";
            if (drawButton(0.5f, startV - 1 * stepV, std::string(dd) + "  Initial conditions", 0.90f,
                Alignment::CENTER, false, condSelected) || isEnterOn(2)) {
                settings_open_ = true;
                selected_ = 2;
            }

            // 3) Help (with [i] symbol)
            const bool helpSelected = (selected_ == 3);
            if (drawButton(0.5f, startV - 2 * stepV, "[i] Help", 0.90f,
                Alignment::CENTER, false, helpSelected) || isEnterOn(3)) {
                help_open_ = true;
                selected_ = 3;
            }

            // 4) Exit
            const bool exitSelected = (selected_ == 4);
            if (drawButton(0.5f, startV - 3 * stepV, "Exit game", 0.90f,
                Alignment::CENTER, false, exitSelected) || isEnterOn(4)) {
                exit_clicked_ = true;
            }
        }
    }

    // =========================
    //  Modal: Initial conditions (RIGHT)
    //  — while open, every other entry is disabled/hidden
    // =========================
    if (settings_open_) {
        static int focus = 0; // 0 = strike force, 1 = friction

        const float panelU = 0.42f;
        const float headV = 0.60f;
        const float row1V = 0.54f;
        const float row2V = 0.49f;
        const float closeV = 0.43f;

        // Header shows the DOWN arrow symbol to indicate open state
        AddText(panelU, headV, "Initial conditions  v", 0.9f, Alignment::LEFT, true);

        const float lineScale = 0.8f;

        // Strike force
        std::string sf = std::format("Strike force: {:.2f}", Config::power_coeff);
        AddText(panelU, row1V, sf, lineScale, Alignment::LEFT, (focus == 0));
        float sfWidthPx = estimateWidthPx(sf, lineScale);
        float leftU = (panelU * winW + sfWidthPx + 18.0f) / winW;
        float rightU = (panelU * winW + sfWidthPx + 48.0f) / winW;
        if (button(leftU, row1V, "<", lineScale, Alignment::CENTER, false, nullptr, nullptr))
            Config::power_coeff -= 0.05f;
        if (button(rightU, row1V, ">", lineScale, Alignment::CENTER, false, nullptr, nullptr))
            Config::power_coeff += 0.05f;

        // Ball friction (mirror to velocity_multiplier)
        std::string bf = std::format("Ball friction: {:.4f}", Config::linear_damping);
        AddText(panelU, row2V, bf, lineScale, Alignment::LEFT, (focus == 1));
        float bfWidthPx = estimateWidthPx(bf, lineScale);
        leftU = (panelU * winW + bfWidthPx + 18.0f) / winW;
        rightU = (panelU * winW + bfWidthPx + 48.0f) / winW;
        if (button(leftU, row2V, "<", lineScale, Alignment::CENTER, false, nullptr, nullptr)) {
            Config::linear_damping = std::max(0.970f, Config::linear_damping - 0.002f);
            Config::velocity_multiplier = Config::linear_damping;
        }
        if (button(rightU, row2V, ">", lineScale, Alignment::CENTER, false, nullptr, nullptr)) {
            Config::linear_damping = std::min(0.999f, Config::linear_damping + 0.002f);
            Config::velocity_multiplier = Config::linear_damping;
        }

        // Close
        if (drawButton(panelU, closeV, "Close [Esc]", 0.8f, Alignment::LEFT, false, false)) {
            settings_open_ = false;
            selected_ = has_started ? 2 : 1; // restore to the "Initial conditions" slot
        }

        // Modal keyboard capture
        static int prevUp = GLFW_RELEASE, prevDown = GLFW_RELEASE, prevLeft = GLFW_RELEASE, prevRight = GLFW_RELEASE, prevEsc = GLFW_RELEASE;
        int kUp = glfwGetKey(w, GLFW_KEY_UP);
        int kDown = glfwGetKey(w, GLFW_KEY_DOWN);
        int kLeft = glfwGetKey(w, GLFW_KEY_LEFT);
        int kRight = glfwGetKey(w, GLFW_KEY_RIGHT);
        int kEsc = glfwGetKey(w, GLFW_KEY_ESCAPE);

        if (kUp == GLFW_PRESS && prevUp == GLFW_RELEASE) focus = 0;
        if (kDown == GLFW_PRESS && prevDown == GLFW_RELEASE) focus = 1;

        if (kLeft == GLFW_PRESS && prevLeft == GLFW_RELEASE) {
            if (focus == 0) Config::power_coeff -= 0.05f;
            else {
                Config::linear_damping = std::max(0.970f, Config::linear_damping - 0.002f);
                Config::velocity_multiplier = Config::linear_damping;
            }
        }
        if (kRight == GLFW_PRESS && prevRight == GLFW_RELEASE) {
            if (focus == 0) Config::power_coeff += 0.05f;
            else {
                Config::linear_damping = std::min(0.999f, Config::linear_damping + 0.002f);
                Config::velocity_multiplier = Config::linear_damping;
            }
        }
        if (kEsc == GLFW_PRESS && prevEsc == GLFW_RELEASE) {
            settings_open_ = false;
            selected_ = has_started ? 2 : 1;
        }

        prevUp = kUp; prevDown = kDown; prevLeft = kLeft; prevRight = kRight; prevEsc = kEsc;
    }

    // =========================
    //  Modal: HELP (CENTER)
    // =========================
    if (help_open_) {
        // Header shows the [i] symbol as requested
        AddText(0.5f, 0.62f, "[i] Help", 1.1f, Alignment::CENTER, true);

        RenderHelpBlock(*this,
            /*u*/0.5f,
            /*vTop*/0.56f,
            /*align*/Alignment::CENTER,
            /*scale*/0.75f,
            /*lineGap*/0.045f);

        if (drawButton(0.5f, 0.56f - 7 * 0.045f - 0.05f, "Close [Esc]", 0.8f, Alignment::CENTER, false, false)) {
            help_open_ = false;
            selected_ = has_started ? 3 : 2; // go back to Help slot
        }

        // Esc to close (modal capture)
        static int prevEscHelp = GLFW_RELEASE;
        int kEsc = glfwGetKey(w, GLFW_KEY_ESCAPE);
        if (kEsc == GLFW_PRESS && prevEscHelp == GLFW_RELEASE) {
            help_open_ = false;
            selected_ = has_started ? 3 : 2;
        }
        prevEscHelp = kEsc;
    }

    // update click-edge trackers
    prevMouse = curMouse;
    prevEnter = curEnter;
}

bool Menu::ConsumePlayClicked() { bool b = play_clicked_;  play_clicked_ = false; return b; }
bool Menu::ConsumeExitClicked() { bool b = exit_clicked_;  exit_clicked_ = false; return b; }
bool Menu::ConsumeResetClicked() { bool b = reset_clicked_; reset_clicked_ = false; return b; }

void Menu::ControlState()
{
    GLFWwindow* window = glfwGetCurrentContext();

    // DOWN: move selection down; if nothing selected yet, start at 0
    const int down_state = glfwGetKey(window, GLFW_KEY_DOWN);
    if (down_state == GLFW_PRESS && last_down_state_ != GLFW_PRESS) {
        if (selected_ == -1) selected_ = 0; else selected_++;
    }
    last_down_state_ = down_state;

    // UP: move selection up; if nothing selected yet, start at 0
    const int up_state = glfwGetKey(window, GLFW_KEY_UP);
    if (up_state == GLFW_PRESS && last_up_state_ != GLFW_PRESS) {
        if (selected_ == -1) selected_ = 0; else selected_--;
    }
    last_up_state_ = up_state;
}

void Menu::AddText(const float u, const float v, const std::string& text,
    const float scale, Alignment alignment, const bool selected)
{
    texts_.emplace_back(u * width_, v * height_, text, scale, alignment, selected);
}